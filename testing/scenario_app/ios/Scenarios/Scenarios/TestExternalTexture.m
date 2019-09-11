//
//  TestExternalTexture.m
//  Scenarios
//
//  Created by lujunchen on 2019/8/30.
//  Copyright © 2019 flutter. All rights reserved.
//

#import "TestExternalTexture.h"
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#define SHADER_STRING(text) @ #text

NSString* const kIFGLGeneralVertexShaderString =
    SHADER_STRING(attribute vec4 position; attribute vec4 inputTextureCoordinate;

                  varying vec2 textureCoordinate;

                  void main() {
                    gl_Position = position;
                    textureCoordinate = inputTextureCoordinate.xy;
                  });

NSString* const kIFGLGeneralFragmentShaderString =
    SHADER_STRING(varying highp vec2 textureCoordinate;

                  uniform sampler2D inputImageTexture;

                  void main() { gl_FragColor = texture2D(inputImageTexture, textureCoordinate); });

const GLfloat vetex1[] = {
    -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
};

const GLfloat vetex2[] = {
    -1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
};

const GLfloat vetex3[] = {
    0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
};

const GLfloat vetex4[] = {
    0.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,
};

const GLfloat vetex[] = {
    -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
};

const GLfloat textureGeneralTexCoord[] = {
    0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
};

@interface TestExternalTexture ()
@property(nonatomic, assign) GLuint program;
@property(nonatomic, assign) GLuint vertShader;
@property(nonatomic, assign) GLuint fragShader;
@property(nonatomic, strong) NSMutableArray* attributes;
@property(nonatomic, strong) NSMutableDictionary<NSString*, NSNumber*>* uniforms;
@property(nonatomic, assign) GLuint positionAttributeLocation;
@property(nonatomic, assign) GLuint texCoordAttributeLocation;

@property(nonatomic, strong) dispatch_queue_t displayQueue;
@property(nonatomic, strong) EAGLContext* glContext;

@property(nonatomic, assign) GLuint frameBuffer;
@property(nonatomic, assign) GLuint frameTexture;
@property(nonatomic, assign) GLuint materialTexture;

@property(strong, nonatomic) NSObject<FlutterTextureRegistry>* registry;
@property(strong, nonatomic) NSObject<FlutterBinaryMessenger>* messenger;
@end

@implementation TestExternalTexture
- (instancetype)initWithWithRegistrar:(NSObject<FlutterPluginRegistrar>*)registrar {
  if ((self = [super init])) {
    self.registry = [registrar textures];
    self.messenger = [registrar messenger];

    self.attributes = [[NSMutableArray alloc] init];
    self.uniforms = [[NSMutableDictionary alloc] init];

    self.displayQueue = dispatch_queue_create("external.testqueue", nil);
  }

  return self;
}

- (GLuint)copyShareTexture {
  if (self.frameTexture != 0) {
    return self.frameTexture;
  }
  return 0;
}

- (void)initProgramm {
  self.program = glCreateProgram();

  if (![self compileShader:&_vertShader
                      type:GL_VERTEX_SHADER
                    string:kIFGLGeneralVertexShaderString]) {
    NSLog(@"FMAVEffect FMAVEffectGLProgram Failed to compile vertex shader");
  }

  if (![self compileShader:&_fragShader
                      type:GL_FRAGMENT_SHADER
                    string:kIFGLGeneralFragmentShaderString]) {
    NSLog(@"FMAVEffect FMAVEffectGLProgram Failed to compile fragment shader");
  }

  glAttachShader(self.program, _vertShader);
  glAttachShader(self.program, _fragShader);

  // called before program link
  [self addAttribute:@"position"];
  [self addAttribute:@"inputTextureCoordinate"];

  if (![self link]) {
    NSLog(@"FMAVEffect FMAVEffectGLProgram link failed");
  }
  // 4
  self.positionAttributeLocation = [self attributeIndex:@"position"];
  self.texCoordAttributeLocation = [self attributeIndex:@"inputTextureCoordinate"];
}

- (void)startWithID:(int64_t)textureID {
  int width = [UIScreen mainScreen].bounds.size.width;
  int height = [UIScreen mainScreen].bounds.size.height;

  dispatch_async(self.displayQueue, ^{
    if (self.glContext == NULL) {
      EAGLSharegroup* flutterShareGroup = [self.registry getShareGroup];
      if (flutterShareGroup != NULL) {
        self.glContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3
                                               sharegroup:flutterShareGroup];
      } else {
        self.glContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
      }
    }
    [EAGLContext setCurrentContext:self.glContext];
    if (self.program == 0) {
      [self initProgramm];
    }
    if (self.frameTexture == 0) {
      self.frameTexture = [self createTextureWithWidth:width andHeight:height];
    }
    if (self.frameBuffer == 0) {
      glGenFramebuffers(1, &self->_frameBuffer);
      glBindFramebuffer(GL_FRAMEBUFFER, self.frameBuffer);
    }
    if (self.materialTexture == 0) {
      glGenTextures(1, &self->_materialTexture);
      UIImage* materialImage = [UIImage imageNamed:@"flutter.png"];
      if (materialImage != NULL) {
        [self convertCGImage:materialImage.CGImage
                   toTexture:self.materialTexture
                      inSize:materialImage.size];
      }
    }

    glViewport(0, 0, (int)width, (int)height);

    glBindFramebuffer(GL_FRAMEBUFFER, self.frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, self.frameTexture,
                           0);

    [self useProgramm];

    [self renderTexture:self.materialTexture withVertex:(GLvoid*)vetex];

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    [self.registry textureFrameAvailable:textureID];
  });
}

- (void)convertCGImage:(CGImageRef)image toTexture:(GLuint)textureID inSize:(CGSize)size {
  CGImageRef cgImageRef = image;
  GLuint width = size.width;
  GLuint height = size.height;
  CGRect rect = CGRectMake(0, 0, width, height);
  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
  void* imageData = malloc(width * height * 4);
  CGContextRef context =
      CGBitmapContextCreate(imageData, width, height, 8, width * 4, colorSpace,
                            kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
  if (context == NULL) {
    return;
  }
  CGContextTranslateCTM(context, 0, 0);
  CGContextScaleCTM(context, 1.0f, 1.0f);
  CGColorSpaceRelease(colorSpace);
  CGContextClearRect(context, rect);
  CGContextDrawImage(context, rect, cgImageRef);
  glEnable(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
  glBindTexture(GL_TEXTURE_2D, 0);
  CGContextRelease(context);
  free(imageData);
}

- (void)renderTexture:(GLuint)srcTexture withVertex:(GLvoid*)vertex {
  glVertexAttribPointer(self.positionAttributeLocation, 2, GL_FLOAT, 0, 0, vertex);
  glEnableVertexAttribArray(self.positionAttributeLocation);

  glVertexAttribPointer(self.texCoordAttributeLocation, 2, GL_FLOAT, 0, 0, textureGeneralTexCoord);
  glEnableVertexAttribArray(self.texCoordAttributeLocation);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, srcTexture);
  glUniform1i([self uniformIndex:@"inputImageTexture"], 0);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glFlush();
  glBindTexture(GL_TEXTURE_2D, 0);
}

- (GLuint)createTextureWithWidth:(size_t)width andHeight:(size_t)height {
  GLuint textureID = -1;
  glActiveTexture(GL_TEXTURE1);
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)width, (int)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  return textureID;
}

#pragma mark OPENGL
- (BOOL)compileShader:(GLuint*)shader type:(GLenum)type string:(NSString*)shaderString {
  GLint status;
  const GLchar* source;

  source = (GLchar*)[shaderString UTF8String];
  if (!source) {
    NSLog(@"FMAVEffect FMAVEffectGLProgram Failed to load shader source");
    return NO;
  }

  *shader = glCreateShader(type);
  glShaderSource(*shader, 1, &source, NULL);
  glCompileShader(*shader);

  glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);

  if (status != GL_TRUE) {
    GLint logLength;
    glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
      GLchar* log = (GLchar*)malloc(logLength);
      glGetShaderInfoLog(*shader, logLength, &logLength, log);
      if (shader == &_vertShader) {
        NSLog(@"FMAVEffect FMAVEffectGLProgram compile vertext shader error: %s", log);
      } else {
        NSLog(@"FMAVEffect FMAVEffectGLProgram compile fragment shader error: %s", log);
      }

      free(log);
    }
  }

  return status == GL_TRUE;
}

- (void)addAttribute:(NSString*)attributeName {
  if (![self.attributes containsObject:attributeName]) {
    [self.attributes addObject:attributeName];
    glBindAttribLocation(self.program, (GLuint)[self.attributes indexOfObject:attributeName],
                         [attributeName UTF8String]);
  }
}

- (GLuint)attributeIndex:(NSString*)attributeName {
  return (GLuint)[self.attributes indexOfObject:attributeName];
}

- (GLuint)uniformIndex:(NSString*)uniformName {
  if ([self.uniforms.allKeys containsObject:uniformName]) {
    return (GLuint)[self.uniforms[uniformName] unsignedIntValue];
  }
  int loc = glGetUniformLocation(self.program, [uniformName UTF8String]);
  _uniforms[uniformName] = [NSNumber numberWithInt:loc];
  return loc;
}

- (BOOL)link {
  GLint status;

  glLinkProgram(self.program);

  glGetProgramiv(self.program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE)
    return NO;

  if (self.vertShader) {
    glDeleteShader(self.vertShader);
    self.vertShader = 0;
  }
  if (self.fragShader) {
    glDeleteShader(self.fragShader);
    self.fragShader = 0;
  }
  return YES;
}

- (void)useProgramm {
  glUseProgram(self.program);
}

- (void)dealloc {
  if (_vertShader != 0)
    glDeleteShader(_vertShader);

  if (_fragShader != 0)
    glDeleteShader(_fragShader);

  if (_program != 0)
    glDeleteProgram(_program);
}

@end
