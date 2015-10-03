import 'package:sky/rendering.dart';
import 'package:sky/widgets.dart';

void main() {
  runApp(new MimicExample());
}

class MimicExample extends StatelessComponent {
  Widget build(BuildContext context) {
    MimicOverlay mimicOverlay = new MimicOverlay();
    final stackElements = <Widget>[
      mimicOverlay,
      new Block([
          new Column([
            createChild(mimicOverlay, new Color(0xFFFFFF00)),
            createChild(mimicOverlay, new Color(0xFF00FFFF)),
            createChild(mimicOverlay, new Color(0xFFFF00FF)),
            createChild(mimicOverlay, new Color(0xFFFF0F0F)),
            createChild(mimicOverlay, new Color(0xFFFFFFFF)),
            createChild(mimicOverlay, new Color(0xFF808080)),
            createChild(mimicOverlay, new Color(0xFFFF0000)),
            createChild(mimicOverlay, new Color(0xFF00FF00)),
            createChild(mimicOverlay, new Color(0xFF0000FF))
          ])
        ],
        onScroll: (offset) { mimicOverlay.currentState.scrollerMoved(offset); }
      )
    ];
    return new GestureDetector(onTap: () {
      mimicOverlay.currentState.setActiveOverlay(null);
    }, child: new Stack(stackElements));
  }

  Widget createChild(MimicOverlay mimicOverlay, Color color) {
    GlobalKey key = new GlobalKey();
    return new GestureDetector(onTap: () {
      mimicOverlay.currentState.setActiveOverlay(key);
    },
        child: new Mimicable(
            key: key,
            child: new Container(
                width: 100.0,
                height: 100.0,
                decoration: new BoxDecoration(backgroundColor: color))));
  }
}

class MimicOverlay extends UniqueComponent<MimicOverlayState> {
  MimicOverlay(): super(key: new GlobalKey());
  MimicOverlayState createState() => new MimicOverlayState();
}

class MimicOverlayState extends State<MimicOverlay> {
  MimicableKey _activeOverlay;

  void setActiveOverlay(GlobalKey mimicableKey) {
    setState(() {
      _activeOverlay?.stopMimic();
      _activeOverlay = null;
      if (mimicableKey != null) {
        _activeOverlay =
            (mimicableKey.currentState as MimicableState).startMimic();
      }
    });
  }

  Rect _getMimicBounds() {
    Rect globalBounds = _activeOverlay.globalBounds;
    assert(globalBounds != null);

    return (context.findRenderObject().parent as RenderBox)
            .globalToLocal(globalBounds.topLeft) &
        globalBounds.size;
  }

  bool _scrolled = false;
  void scrollerMoved(double offset) {
    setState(() { _scrolled = true; });
  }

  Widget build(BuildContext context) {
    if (_activeOverlay != null) {
      if (_scrolled) {
        _scrolled = false;
        WidgetFlutterBinding.instance.rebuildAndRelayoutAllOverAgainBeforePainting(this);
      }
      Rect mimicBounds = _getMimicBounds();
      return new Container(
          child: new Positioned(
              left: mimicBounds.left - 100.0,
              top: mimicBounds.top,
              child: new SizedBox(
                  width: mimicBounds.width,
                  height: mimicBounds.height,
                  child: new Mimic(original: _activeOverlay))));
    }

    return new Container(width: 0.0, height: 0.0);
  }
}