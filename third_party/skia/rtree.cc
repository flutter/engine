/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <list>

#include "rtree.h"

RTree::RTree() : fCount(0) {}

void RTree::insert(const SkRect boundsArray[], const SkBBoxHierarchy::Metadata metadata[], int N) {
    SkASSERT(0 == fCount);

    std::vector<Branch> branches;
    branches.reserve(N);

    for (int i = 0; i < N; i++) {
        const SkRect& bounds = boundsArray[i];
        if (bounds.isEmpty()) {
            continue;
        }
        Branch b;
        b.fBounds = bounds;
        b.fOpIndex = i;
        b.isDraw = (metadata == nullptr) ? false : metadata[i].isDraw;
        branches.push_back(b);
    }

    fCount = (int)branches.size();
    if (fCount) {
        if (1 == fCount) {
            fNodes.reserve(1);
            Node* n = this->allocateNodeAtLevel(0);
            n->fNumChildren = 1;
            n->fChildren[0] = branches[0];
            fRoot.fSubtree = n;
            fRoot.fBounds = branches[0].fBounds;
        } else {
            fNodes.reserve(CountNodes(fCount));
            fRoot = this->bulkLoad(&branches);
        }
    }
}

void RTree::insert(const SkRect boundsArray[], int N) { insert(boundsArray, nullptr, N); }

RTree::Node* RTree::allocateNodeAtLevel(uint16_t level) {
    SkDEBUGCODE(Node* p = fNodes.data());
    fNodes.push_back(Node{});
    Node& out = fNodes.back();
    SkASSERT(fNodes.data() == p);  // If this fails, we didn't reserve() enough.
    out.fNumChildren = 0;
    out.fLevel = level;
    return &out;
}

// This function parallels bulkLoad, but just counts how many nodes bulkLoad
// would allocate.
int RTree::CountNodes(int branches) {
    if (branches == 1) {
        return 1;
    }
    int numBranches = branches / kMaxChildren;
    int remainder = branches % kMaxChildren;
    if (remainder > 0) {
        numBranches++;
        if (remainder >= kMinChildren) {
            remainder = 0;
        } else {
            remainder = kMinChildren - remainder;
        }
    }
    int currentBranch = 0;
    int nodes = 0;
    while (currentBranch < branches) {
        int incrementBy = kMaxChildren;
        if (remainder != 0) {
            if (remainder <= kMaxChildren - kMinChildren) {
                incrementBy -= remainder;
                remainder = 0;
            } else {
                incrementBy = kMinChildren;
                remainder -= kMaxChildren - kMinChildren;
            }
        }
        nodes++;
        currentBranch++;
        for (int k = 1; k < incrementBy && currentBranch < branches; ++k) {
            currentBranch++;
        }
    }
    return nodes + CountNodes(nodes);
}

RTree::Branch RTree::bulkLoad(std::vector<Branch>* branches, int level) {
    if (branches->size() == 1) {  // Only one branch.  It will be the root.
        return (*branches)[0];
    }

    // We might sort our branches here, but we expect Blink gives us a reasonable
    // x,y order. Skipping a call to sort (in Y) here resulted in a 17% win for
    // recording with negligible difference in playback speed.
    int numBranches = (int)branches->size() / kMaxChildren;
    int remainder = (int)branches->size() % kMaxChildren;
    int newBranches = 0;

    if (remainder > 0) {
        ++numBranches;
        // If the remainder isn't enough to fill a node, we'll add fewer nodes to
        // other branches.
        if (remainder >= kMinChildren) {
            remainder = 0;
        } else {
            remainder = kMinChildren - remainder;
        }
    }

    int currentBranch = 0;
    while (currentBranch < (int)branches->size()) {
        int incrementBy = kMaxChildren;
        if (remainder != 0) {
            // if need be, omit some nodes to make up for remainder
            if (remainder <= kMaxChildren - kMinChildren) {
                incrementBy -= remainder;
                remainder = 0;
            } else {
                incrementBy = kMinChildren;
                remainder -= kMaxChildren - kMinChildren;
            }
        }
        Node* n = allocateNodeAtLevel(level);
        n->fNumChildren = 1;
        n->fChildren[0] = (*branches)[currentBranch];
        Branch b;
        b.fBounds = (*branches)[currentBranch].fBounds;
        b.fSubtree = n;
        ++currentBranch;
        for (int k = 1; k < incrementBy && currentBranch < (int)branches->size(); ++k) {
            b.fBounds.join((*branches)[currentBranch].fBounds);
            n->fChildren[k] = (*branches)[currentBranch];
            ++n->fNumChildren;
            ++currentBranch;
        }
        (*branches)[newBranches] = b;
        ++newBranches;
    }
    branches->resize(newBranches);
    return this->bulkLoad(branches, level + 1);
}

void RTree::search(const SkRect& query, std::vector<int>* results) const {
    if (fCount > 0 && SkRect::Intersects(fRoot.fBounds, query)) {
        this->search(fRoot.fSubtree, query, results);
    }
}

void RTree::search(Node* node, const SkRect& query, std::vector<int>* results) const {
    for (int i = 0; i < node->fNumChildren; ++i) {
        if (!SkRect::Intersects(node->fChildren[i].fBounds, query)) {
            continue;
        }
        if (0 != node->fLevel) {
            this->search(node->fChildren[i].fSubtree, query, results);
            continue;
        }
        results->push_back(node->fChildren[i].fOpIndex);
    }
}

std::list<SkRect> RTree::searchNonOverlappingDrawnRects(const SkRect& query) const {
    std::list<SkRect> results;
    if (fCount > 0 && SkRect::Intersects(fRoot.fBounds, query)) {
        this->searchNonOverlappingDrawnRects(fRoot.fSubtree, query, results);
    }
    return results;
}

void RTree::searchNonOverlappingDrawnRects(Node* node,
                                           const SkRect& query,
                                           std::list<SkRect>& results) const {
    if (!SkRect::Intersects(fRoot.fBounds, query)) {
        return;
    }
    for (int i = 0; i < node->fNumChildren; ++i) {
        if (!SkRect::Intersects(node->fChildren[i].fBounds, query)) {
            continue;
        }
        // Non-leaf node
        if (0 != node->fLevel) {
            this->searchNonOverlappingDrawnRects(node->fChildren[i].fSubtree, query, results);
            continue;
        }
        // Ignore records that don't draw anything.
        if (!node->fChildren[i].isDraw) {
            continue;
        }
        SkRect currentRecordRect = node->fChildren[i].fBounds;
        bool replacedExistingRect = false;
        // // If the current record rect intersects with any of the rects in the
        // // result list, then join them, and update the rect in results.
        std::list<SkRect>::iterator currRectItr = results.begin();
        std::list<SkRect>::iterator firstIntersectingRectItr;
        while (!replacedExistingRect && currRectItr != results.end()) {
            if (SkRect::Intersects(*currRectItr, currentRecordRect)) {
                replacedExistingRect = true;
                firstIntersectingRectItr = currRectItr;
                currRectItr->join(currentRecordRect);
            }
            currRectItr++;
        }
        // It's possible that the result contains duplicated rects at this point.
        // For example, consider a result list that contains rects A, B. If a
        // new rect C is a superset of A and B, then A and B are the same set after
        // the merge. As a result, find such cases and remove them from the result list.
        while (replacedExistingRect && currRectItr != results.end()) {
            if (SkRect::Intersects(*currRectItr, *firstIntersectingRectItr)) {
                firstIntersectingRectItr->join(*currRectItr);
                results.erase(currRectItr);
            }
            currRectItr++;
        }
        if (!replacedExistingRect) {
            results.push_back(currentRecordRect);
        }
    }
}

size_t RTree::bytesUsed() const {
    size_t byteCount = sizeof(RTree);
    byteCount += fNodes.capacity() * sizeof(Node);
    return byteCount;
}

RTreeFactory::RTreeFactory() { r_tree_ = sk_make_sp<RTree>(); }

sk_sp<RTree> RTreeFactory::getInstance() { return r_tree_; }

sk_sp<SkBBoxHierarchy> RTreeFactory::operator()() const { return r_tree_; }
