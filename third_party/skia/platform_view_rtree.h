/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKIA_PLATFORM_VIEW_RTREE_H_
#define SKIA_PLATFORM_VIEW_RTREE_H_

#include <list>

#include "third_party/skia/include/core/SkBBHFactory.h"
#include "third_party/skia/include/core/SkTypes.h"

/**
 * An R-Tree implementation. In short, it is a balanced n-ary tree containing a hierarchy of
 * bounding rectangles.
 *
 * It only supports bulk-loading, i.e. creation from a batch of bounding rectangles.
 * This performs a bottom-up bulk load using the STR (sort-tile-recursive) algorithm.
 *
 * TODO: Experiment with other bulk-load algorithms (in particular the Hilbert pack variant,
 * which groups rects by position on the Hilbert curve, is probably worth a look). There also
 * exist top-down bulk load variants (VAMSplit, TopDownGreedy, etc).
 *
 * For more details see:
 *
 *  Beckmann, N.; Kriegel, H. P.; Schneider, R.; Seeger, B. (1990). "The R*-tree:
 *      an efficient and robust access method for points and rectangles"
 *
 * The original source code can be found on
 * https://github.com/google/skia/blob/508fd32091afe334d4e1dd6cdaa63168a53acb26/src/core/SkRTree.h
 *
 * This copy includes a searchRects method.
 */
class PlatformViewRTree : public SkBBoxHierarchy {
public:
    PlatformViewRTree();

    void insert(const SkRect[], const SkBBoxHierarchy::Metadata[], int N) override;
    void insert(const SkRect[], int N) override;
    void search(const SkRect& query, std::vector<int>* results) const override;

    // Finds the rects in the tree that represent drawing operations and intersect
    // with the query rect.
    //
    // When two rects intersect with each other, they are joined into a single rect
    // which also intersects with the query rect. In other words, the bounds of each
    // rect in the result list are mutually exclusive.
    //
    // Since this method is used when compositing platform views, the rects in the
    // result list represent UIViews that are composed on top of the platform view.
    //
    // However, Skia uses this tree to tracks operations that don't have any context
    // on how they relate to each other when compositing the final scene in Flutter.
    // For example, they may include matrix changes, clips or rects that are stacked
    // on top of each other.
    std::list<SkRect> searchRects(const SkRect& query) const;

    size_t bytesUsed() const override;

    // Methods and constants below here are only public for tests.

    // Return the depth of the tree structure.
    int getDepth() const { return fCount ? fRoot.fSubtree->fLevel + 1 : 0; }
    // Insertion count (not overall node count, which may be greater).
    int getCount() const { return fCount; }

    // These values were empirically determined to produce reasonable performance
    // in most cases.
    static const int kMinChildren = 6, kMaxChildren = 11;

private:
    struct Node;

    struct Branch {
        union {
            Node* fSubtree;
            int fOpIndex;
        };
        // True if the current bounds represent a drawing operation.
        bool isDraw;
        SkRect fBounds;
    };

    struct Node {
        uint16_t fNumChildren;
        uint16_t fLevel;
        Branch fChildren[kMaxChildren];
    };

    void search(Node* root, const SkRect& query, std::vector<int>* results) const;
    void searchRects(Node* root, const SkRect& query, std::list<SkRect>& results) const;

    // Consumes the input array.
    Branch bulkLoad(std::vector<Branch>* branches, int level = 0);

    // How many times will bulkLoad() call allocateNodeAtLevel()?
    static int CountNodes(int branches);

    Node* allocateNodeAtLevel(uint16_t level);

    // This is the count of data elements (rather than total nodes in the tree)
    int fCount;
    Branch fRoot;
    std::vector<Node> fNodes;
};

class PlatformViewRTreeFactory : public SkBBHFactory {
public:
    PlatformViewRTreeFactory(sk_sp<PlatformViewRTree>& r_tree);
    sk_sp<SkBBoxHierarchy> operator()() const override;

private:
    sk_sp<PlatformViewRTree> r_tree_;
};

#endif  // SKIA_PLATFORM_VIEW_RTREE_H_
