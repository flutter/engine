package io.flutter.embedding.engine.mutatorsstack;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Path;
import android.widget.FrameLayout;
import androidx.annotation.NonNull;

/**
 * A view that applies the {@link io.flutter.embedding.engine.mutatorsstack.MutatorsStack} to its
 * children.
 */
public class FlutterMutatorView extends FrameLayout {
  private FlutterMutatorsStack mutatorsStack;
  private float screenDensity;
  private int left;
  private int top;

  /** Initialize the FlutterMutatorView. */
  public FlutterMutatorView(@NonNull Context context, float screenDensity) {
    super(context, null);
    this.screenDensity = screenDensity;
  }

  /**
   * Pass the necessary parameters to the view so it can apply correct mutations to its children.
   */
  public void readyToDisplay(
      @NonNull FlutterMutatorsStack mutatorsStack, int left, int top, int width, int height) {
    this.mutatorsStack = mutatorsStack;
    this.left = left;
    this.top = top;
    FrameLayout.LayoutParams layoutParams = new FrameLayout.LayoutParams(width, height);
    layoutParams.leftMargin = left;
    layoutParams.topMargin = top;
    setLayoutParams(layoutParams);
    setWillNotDraw(false);
  }

  @Override
  public void draw(Canvas canvas) {
    // Apply all clippings on the parent canvas.
    canvas.save();
    for (Path path : mutatorsStack.getFinalClippingPaths()) {
      // Reverse the current offset.
      //
      // The frame of this view includes the final offset of the bounding rect.
      // We need to apply all the mutators to the view, which includes the mutation that leads to
      // the final offset. We should reverse this final offset, both as a translate mutation and to
      // all the clipping paths
      Path pathCopy = new Path(path);
      pathCopy.offset(-left, -top);
      canvas.clipPath(pathCopy);
    }
    super.draw(canvas);
    canvas.restore();
  }

  @Override
  public void dispatchDraw(Canvas canvas) {
    // Apply all the transforms on the child canvas.
    canvas.save();
    // Reverse the current offset.
    //
    // The frame of this view includes the final offset of the bounding rect.
    // We need to apply all the mutators to the view, which includes the mutation that leads to
    // the final offset. We should reverse this final offset, both as a translate mutation and to
    // all the clipping paths
    Matrix reverseTranslateMatrix = new Matrix();
    reverseTranslateMatrix.preTranslate(-left, -top);

    // Reverse scale based on screen scale.
    //
    // The Android frame is set based on the logical resolution instead of physical.
    // (https://developer.android.com/training/multiscreen/screendensities).
    // However, flow is based on the physical resolution. For example, 1000 pixels in flow equals
    // 500 points in Android. And until this point, we did all the calculation based on the flow
    // resolution. So we need to scale down to match Android's logical resolution.
    Matrix reverseScaleMatrix = new Matrix();
    reverseScaleMatrix.preScale(1 / screenDensity, 1 / screenDensity);

    Matrix finalMatrix = new Matrix(mutatorsStack.getFinalMatrix());
    finalMatrix.postConcat(reverseTranslateMatrix);
    finalMatrix.preConcat(reverseScaleMatrix);
    canvas.concat(finalMatrix);
    super.dispatchDraw(canvas);
    canvas.restore();
  }
}
