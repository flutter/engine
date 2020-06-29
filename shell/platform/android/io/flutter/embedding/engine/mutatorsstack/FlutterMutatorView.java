package io.flutter.embedding.engine.mutatorsstack;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Path;
import android.graphics.Rect;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import androidx.annotation.NonNull;

import java.util.List;

public class FlutterMutatorView extends FrameLayout {

    private FlutterMutatorsStack mutatorsStack;
    private float screenDensity;

    public FlutterMutatorView(@NonNull FlutterMutatorsStack mutatorsStack, @NonNull Context context, float screenDensity) {
        super(context, null);
        this.mutatorsStack = mutatorsStack;
        this.screenDensity = screenDensity;
    }

    @Override
    public void draw(Canvas canvas) {
        canvas.save();
        // Reverse the current offset.
        //
        // The frame of this view includes the final offset of the bounding rect.
        // We need to apply all the mutators to the view, which includes the mutation that leads to
        // the final offset. We should reverse this final offset, both as a translate mutation and to all the clipping paths
        FrameLayout.LayoutParams layoutParams = (FrameLayout.LayoutParams)getLayoutParams();
        Matrix reverseTranslateMatrix = new Matrix();
        reverseTranslateMatrix.preTranslate(-layoutParams.leftMargin, -layoutParams.topMargin);

        // Reverse scale based on screen scale.
        //
        // The Android frame is set based on the logical resolution instead of physical.
        // (https://developer.android.com/training/multiscreen/screendensities).
        // However, flow is based on the physical resolution. For example, 1000 pixels in flow equals
        // 500 points in Android. And until this point, we did all the calculation based on the flow
        // resolution. So we need to scale down to match Android's logical resolution.
        Matrix reverseScaleMatrix = new Matrix();
        reverseScaleMatrix.preScale(1/screenDensity, 1/screenDensity);

        float[] matrixValues = new float[9];
        // for (Path path: mutatorsStack.getFinalClippingPaths()) {
        //     // for ()
        //     // path.transform(finalReverseMatrix);
        //     canvas.clipPath(path);
        //     io.flutter.Log.e("path ", "path " + path);
        // }
        Matrix finalMatrix = mutatorsStack.getFinalMatrix();
        finalMatrix.getValues(matrixValues);
        io.flutter.Log.e("matrix ", "----------------- final matrix -----------------");
        io.flutter.Log.e("matrix ", "MTRANS_X " + matrixValues[2]);
        io.flutter.Log.e("matrix ", "MTRANS_Y " + matrixValues[5]);
        io.flutter.Log.e("matrix ", "MSKEW_X " + matrixValues[1]);
        io.flutter.Log.e("matrix ", "MSKEW_Y " + matrixValues[3]);
        io.flutter.Log.e("matrix ", "MSCALE_X " + matrixValues[0]);
        io.flutter.Log.e("matrix ", "MSCALE_Y " + matrixValues[4]);
        io.flutter.Log.e("matrix ", "MPERSP_X " + matrixValues[7]);
        io.flutter.Log.e("matrix ", "MPERSP_Y " + matrixValues[8]);
        io.flutter.Log.e("matrix ", "MPERSP_Z " + matrixValues[6]);
        finalMatrix.postConcat(reverseTranslateMatrix);
        finalMatrix.getValues(matrixValues);
        io.flutter.Log.e("matrix ", "----------------- final matrix reverse translate -----------------");
        io.flutter.Log.e("matrix ", "MTRANS_X " + matrixValues[2]);
        io.flutter.Log.e("matrix ", "MTRANS_Y " + matrixValues[5]);
        io.flutter.Log.e("matrix ", "MSKEW_X " + matrixValues[1]);
        io.flutter.Log.e("matrix ", "MSKEW_Y " + matrixValues[3]);
        io.flutter.Log.e("matrix ", "MSCALE_X " + matrixValues[0]);
        io.flutter.Log.e("matrix ", "MSCALE_Y " + matrixValues[4]);
        io.flutter.Log.e("matrix ", "MPERSP_X " + matrixValues[7]);
        io.flutter.Log.e("matrix ", "MPERSP_Y " + matrixValues[8]);
        io.flutter.Log.e("matrix ", "MPERSP_Z " + matrixValues[6]);
        finalMatrix.preConcat(reverseScaleMatrix);
        finalMatrix.getValues(matrixValues);
        io.flutter.Log.e("matrix ", "----------------- final matrix reverse scale -----------------");
        io.flutter.Log.e("matrix ", "MTRANS_X " + matrixValues[2]);
        io.flutter.Log.e("matrix ", "MTRANS_Y " + matrixValues[5]);
        io.flutter.Log.e("matrix ", "MSKEW_X " + matrixValues[1]);
        io.flutter.Log.e("matrix ", "MSKEW_Y " + matrixValues[3]);
        io.flutter.Log.e("matrix ", "MSCALE_X " + matrixValues[0]);
        io.flutter.Log.e("matrix ", "MSCALE_Y " + matrixValues[4]);
        io.flutter.Log.e("matrix ", "MPERSP_X " + matrixValues[7]);
        io.flutter.Log.e("matrix ", "MPERSP_Y " + matrixValues[8]);
        io.flutter.Log.e("matrix ", "MPERSP_Z " + matrixValues[6]);


        canvas.concat(finalMatrix);
        super.draw(canvas);
        canvas.restore();
    }

    // @Override
    // public void draw (Canvas canvas) {
    //     canvas.save();
    //     // Reverse the current offset.
    //     // The frame of this view includes the final offset of the bounding rect.
    //     // We need to apply all the mutators to the view, which includes the mutation that leads to
    //     // the final offset. We should reverse this final offset first then apply the mutations so after
    //     // all the calculation, the offset of the final drawing is the same as the final offset of the bounding rect.
    //     FrameLayout.LayoutParams layoutParams = (FrameLayout.LayoutParams)getLayoutParams();
    //     canvas.translate(-layoutParams.leftMargin, -layoutParams.topMargin);
    //     mutatorsStack.transformClippings();
    //     List<FlutterMutator> mutators = mutatorsStack.getMutators();
    //     for (int i = mutators.size()-1; i >= 0; i --) {
    //         FlutterMutator mutator = mutators.get(i);
    //         switch(mutator.getType()) {
    //             case TRANSFORM: {
    //                 Matrix matrix = mutator.getMatrix();
    //                 float[] matrixValues = new float[9];
    //                 matrix.getValues(matrixValues);
    //                 io.flutter.Log.e("matrix ", "----------------- concat matrix -----------------");
    //                 io.flutter.Log.e("matrix ", "MTRANS_X " + matrixValues[2]);
    //                 io.flutter.Log.e("matrix ", "MTRANS_Y " + matrixValues[5]);
    //                 io.flutter.Log.e("matrix ", "MSKEW_X " + matrixValues[1]);
    //                 io.flutter.Log.e("matrix ", "MSKEW_Y " + matrixValues[3]);
    //                 io.flutter.Log.e("matrix ", "MSCALE_X " + matrixValues[0]);
    //                 io.flutter.Log.e("matrix ", "MSCALE_Y " + matrixValues[4]);
    //                 io.flutter.Log.e("matrix ", "MPERSP_X " + matrixValues[7]);
    //                 io.flutter.Log.e("matrix ", "MPERSP_Y " + matrixValues[8]);
    //                 io.flutter.Log.e("matrix ", "MPERSP_Z " + matrixValues[6]);
    //                 canvas.concat(mutator.getMatrix());
    //                 break;
    //             }
    //             case CLIP_RECT: {
    //                 io.flutter.Log.e("matrix ", "Clipped rect");
    //                 Rect rect = mutator.getRect();
    //                 canvas.clipRect(rect);
    //                 break;
    //             }
    //             case CLIP_PATH: {
    //                 Path path = mutator.getPath();
    //                 canvas.clipPath(path);
    //                 break;
    //             }
    //             case CLIP_RRECT:
    //             case OPACITY:
    //                 break;
    //         }
    //     }

    //     // Reverse scale based on screen scale.
    //     //
    //     // The Android frame is set based on the logical resolution instead of physical.
    //     // (https://developer.android.com/training/multiscreen/screendensities).
    //     // However, flow is based on the physical resolution. For example, 1000 pixels in flow equals
    //     // 500 points in Android. And until this point, we did all the calculation based on the flow
    //     // resolution. So we need to scale down to match Android's logical resolution.
    //     canvas.scale(1/screenDensity, 1/screenDensity);
    //     super.draw(canvas);
    //     canvas.restore();
    // }

    // private void disableClipping(View view) {
    //     if (view instanceof ViewGroup) {
    //         io.flutter.Log.e("disableClipping ", "disableClipping ");
    //         ViewGroup viewGroup = (ViewGroup)view;
    //         viewGroup.setClipChildren(false);
    //     }
    //     view.setClipToOutline(false);
    // }

    // void applyMutatorsToCanvas(List<FlutterMutator> mutators, int i, Canvas canvas) {
    //     if (i >= mutators.size()) {
    //         return;
    //     }
    //     FlutterMutator mutator = mutators.get(i);
    //     switch(mutator.getType()) {
    //         case TRANSFORM: {
    //             canvas.concat(mutator.getMatrix());
    //             break;
    //         }
    //         case CLIP_RECT: {

    //             break;
    //         }
    //         case CLIP_PATH:
    //         case CLIP_RRECT:
    //         case OPACITY:
    //             break;
    //     }
    //     next(mutators, i+1, canvas);
    // }

}