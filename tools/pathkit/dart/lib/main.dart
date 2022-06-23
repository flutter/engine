import 'pathkit.dart';

void main() {
  final Path path = Path(FillType.nonZero);
  path.lineTo(10, 0);
  path.lineTo(10, 10);
  path.lineTo(0, 10);
  path.close();

  final Path path2 = Path(FillType.nonZero);
  path.moveTo(5, 5);
  path.lineTo(15, 5);
  path.lineTo(15, 15);
  path.lineTo(5, 15);
  path.close();

  path.applyOp(path2, PathOp.difference);
  path.dump();
}