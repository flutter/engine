package io.flutter.plugin.editing;

import static org.mockito.Mockito.isNull;

import android.content.Context;
import android.view.textservice.SpellCheckerSession;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.embedding.engine.systemchannels.SpellCheckChannel;
import io.flutter.plugin.common.BinaryMessenger;
import java.util.Arrays;
import org.mockito.ArgumentCaptor;

public class SpellCheckPluginTest {

  private static void sendToBinaryMessageHandler(
      BinaryMessenger.BinaryMessageHandler binaryMessageHandler, String method, Object args) {
    MethodCall methodCall = new MethodCall(method, args);
    ByteBuffer encodedMethodCall = JSONMethodCodec.INSTANCE.encodeMethodCall(methodCall);
    binaryMessageHandler.onMessage(
        (ByteBuffer) encodedMethodCall.flip(), mock(BinaryMessenger.BinaryReply.class));
  }

  @Test
  public void respondsToSpellCheckChannelMessage() {
    ArgumentCaptor<BinaryMessenger.BinaryMessageHandler> binaryMessageHandlerCaptor =
        ArgumentCaptor.forClass(BinaryMessenger.BinaryMessageHandler.class);
    DartExecutor mockBinaryMessenger = mock(DartExecutor.class);
    SpellCheckChannel.SpellCheckMethodHandler mockHandler =
        mock(SpellCheckChannel.SpellCheckMethodHandler.class);
    SpellCheckChannel spellCheckChannel = new SpellCheckChannel(mockBinaryMessenger);

    spellCheckChannel.setSpellCheckMethodHandler(mockHandler);

    verify(mockBinaryMessenger, times(1))
        .setMessageHandler(any(String.class), binaryMessageHandlerCaptor.capture());

    BinaryMessenger.BinaryMessageHandler binaryMessageHandler =
        binaryMessageHandlerCaptor.getValue();

    sendToBinaryMessageHandler(
        binaryMessageHandler,
        "SpellCheck.initiateSpellCheck",
        Arrays.asList("en-US", "Hello, world!"));
    verify(mockHandler, times(1)).initiateSpellCheck("en-US", "Hello, world!");
  }

  @Test
  public void performSpellCheckRequestsUpdateSpellCheckResults() {
    // Verify call to performSpellCheckResults(...) leads to call to
    // "SpellCheck.updateSpellCheckResults"
    // This would require simulating TextServicesManager because otherwise, there is a lot to mock.
  }

  @Test
  public void onGetSentenceSuggestionsProperlyFormatsSpellCheckResults() {
    // Verify that spell check results are properly formatted by onGetSentenceSuggestions(...)
  }

  @Test
  public void destroyClosesSpellCheckerSessionAndClearsSpellCheckMethodHandler() {
    Context fakeContext = mock(Context.class);
    SpellCheckChannel fakeSpellCheckChannel = mock(SpellCheckChannel.class);
    SpellCheckPlugin spellCheckPlugin = new SpellCheckPlugin(fakeContext, fakeSpellCheckChannel);
    SpellCheckerSesssion fakeSpellCheckerSession = mock(SpellCheckerSession.class);

    spellCheckPlugin.destroy();

    verify(fakeSpellCheckChannel).setSpellCheckMethodHandler(isNull());
    verify(fakeSpellCheckerSession).close();
  }
}
