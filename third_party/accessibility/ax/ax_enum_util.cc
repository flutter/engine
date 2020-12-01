// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ax_enum_util.h"

namespace ax {

const char* ToString(ax::Event event) {
  switch (event) {
    case ax::Event::kNone:
      return "none";
    case ax::Event::kActiveDescendantChanged:
      return "activedescendantchanged";
    case ax::Event::kAlert:
      return "alert";
    case ax::Event::kAriaAttributeChanged:
      return "ariaAttributeChanged";
    case ax::Event::kAutocorrectionOccured:
      return "autocorrectionOccured";
    case ax::Event::kBlur:
      return "blur";
    case ax::Event::kCheckedStateChanged:
      return "checkedStateChanged";
    case ax::Event::kChildrenChanged:
      return "childrenChanged";
    case ax::Event::kClicked:
      return "clicked";
    case ax::Event::kControlsChanged:
      return "controlsChanged";
    case ax::Event::kDocumentSelectionChanged:
      return "documentSelectionChanged";
    case ax::Event::kDocumentTitleChanged:
      return "documentTitleChanged";
    case ax::Event::kEndOfTest:
      return "endOfTest";
    case ax::Event::kExpandedChanged:
      return "expandedChanged";
    case ax::Event::kFocus:
      return "focus";
    case ax::Event::kFocusAfterMenuClose:
      return "focusAfterMenuClose";
    case ax::Event::kFocusContext:
      return "focusContext";
    case ax::Event::kHide:
      return "hide";
    case ax::Event::kHitTestResult:
      return "hitTestResult";
    case ax::Event::kHover:
      return "hover";
    case ax::Event::kImageFrameUpdated:
      return "imageFrameUpdated";
    case ax::Event::kInvalidStatusChanged:
      return "invalidStatusChanged";
    case ax::Event::kLayoutComplete:
      return "layoutComplete";
    case ax::Event::kLiveRegionCreated:
      return "liveRegionCreated";
    case ax::Event::kLiveRegionChanged:
      return "liveRegionChanged";
    case ax::Event::kLoadComplete:
      return "loadComplete";
    case ax::Event::kLoadStart:
      return "loadStart";
    case ax::Event::kLocationChanged:
      return "locationChanged";
    case ax::Event::kMediaStartedPlaying:
      return "mediaStartedPlaying";
    case ax::Event::kMediaStoppedPlaying:
      return "mediaStoppedPlaying";
    case ax::Event::kMenuEnd:
      return "menuEnd";
    case ax::Event::kMenuListItemSelected:
      return "menuListItemSelected";
    case ax::Event::kMenuListValueChanged:
      return "menuListValueChanged";
    case ax::Event::kMenuPopupEnd:
      return "menuPopupEnd";
    case ax::Event::kMenuPopupStart:
      return "menuPopupStart";
    case ax::Event::kMenuStart:
      return "menuStart";
    case ax::Event::kMouseCanceled:
      return "mouseCanceled";
    case ax::Event::kMouseDragged:
      return "mouseDragged";
    case ax::Event::kMouseMoved:
      return "mouseMoved";
    case ax::Event::kMousePressed:
      return "mousePressed";
    case ax::Event::kMouseReleased:
      return "mouseReleased";
    case ax::Event::kRowCollapsed:
      return "rowCollapsed";
    case ax::Event::kRowCountChanged:
      return "rowCountChanged";
    case ax::Event::kRowExpanded:
      return "rowExpanded";
    case ax::Event::kScrollPositionChanged:
      return "scrollPositionChanged";
    case ax::Event::kScrolledToAnchor:
      return "scrolledToAnchor";
    case ax::Event::kSelectedChildrenChanged:
      return "selectedChildrenChanged";
    case ax::Event::kSelection:
      return "selection";
    case ax::Event::kSelectionAdd:
      return "selectionAdd";
    case ax::Event::kSelectionRemove:
      return "selectionRemove";
    case ax::Event::kShow:
      return "show";
    case ax::Event::kStateChanged:
      return "stateChanged";
    case ax::Event::kTextChanged:
      return "textChanged";
    case ax::Event::kTextSelectionChanged:
      return "textSelectionChanged";
    case ax::Event::kTooltipClosed:
      return "tooltipClosed";
    case ax::Event::kTooltipOpened:
      return "tooltipOpened";
    case ax::Event::kWindowActivated:
      return "windowActivated";
    case ax::Event::kWindowDeactivated:
      return "windowDeactivated";
    case ax::Event::kWindowVisibilityChanged:
      return "windowVisibilityChanged";
    case ax::Event::kTreeChanged:
      return "treeChanged";
    case ax::Event::kValueChanged:
      return "valueChanged";
  }

  return "";
}

ax::Event ParseEvent(const char* event) {
  if (0 == strcmp(event, "none"))
    return ax::Event::kNone;
  if (0 == strcmp(event, "activedescendantchanged"))
    return ax::Event::kActiveDescendantChanged;
  if (0 == strcmp(event, "alert"))
    return ax::Event::kAlert;
  if (0 == strcmp(event, "ariaAttributeChanged"))
    return ax::Event::kAriaAttributeChanged;
  if (0 == strcmp(event, "autocorrectionOccured"))
    return ax::Event::kAutocorrectionOccured;
  if (0 == strcmp(event, "blur"))
    return ax::Event::kBlur;
  if (0 == strcmp(event, "checkedStateChanged"))
    return ax::Event::kCheckedStateChanged;
  if (0 == strcmp(event, "childrenChanged"))
    return ax::Event::kChildrenChanged;
  if (0 == strcmp(event, "clicked"))
    return ax::Event::kClicked;
  if (0 == strcmp(event, "controlsChanged"))
    return ax::Event::kControlsChanged;
  if (0 == strcmp(event, "documentSelectionChanged"))
    return ax::Event::kDocumentSelectionChanged;
  if (0 == strcmp(event, "documentTitleChanged"))
    return ax::Event::kDocumentTitleChanged;
  if (0 == strcmp(event, "endOfTest"))
    return ax::Event::kEndOfTest;
  if (0 == strcmp(event, "expandedChanged"))
    return ax::Event::kExpandedChanged;
  if (0 == strcmp(event, "focus"))
    return ax::Event::kFocus;
  if (0 == strcmp(event, "focusAfterMenuClose"))
    return ax::Event::kFocusAfterMenuClose;
  if (0 == strcmp(event, "focusContext"))
    return ax::Event::kFocusContext;
  if (0 == strcmp(event, "hide"))
    return ax::Event::kHide;
  if (0 == strcmp(event, "hitTestResult"))
    return ax::Event::kHitTestResult;
  if (0 == strcmp(event, "hover"))
    return ax::Event::kHover;
  if (0 == strcmp(event, "imageFrameUpdated"))
    return ax::Event::kImageFrameUpdated;
  if (0 == strcmp(event, "invalidStatusChanged"))
    return ax::Event::kInvalidStatusChanged;
  if (0 == strcmp(event, "layoutComplete"))
    return ax::Event::kLayoutComplete;
  if (0 == strcmp(event, "liveRegionCreated"))
    return ax::Event::kLiveRegionCreated;
  if (0 == strcmp(event, "liveRegionChanged"))
    return ax::Event::kLiveRegionChanged;
  if (0 == strcmp(event, "loadComplete"))
    return ax::Event::kLoadComplete;
  if (0 == strcmp(event, "loadStart"))
    return ax::Event::kLoadStart;
  if (0 == strcmp(event, "locationChanged"))
    return ax::Event::kLocationChanged;
  if (0 == strcmp(event, "mediaStartedPlaying"))
    return ax::Event::kMediaStartedPlaying;
  if (0 == strcmp(event, "mediaStoppedPlaying"))
    return ax::Event::kMediaStoppedPlaying;
  if (0 == strcmp(event, "menuEnd"))
    return ax::Event::kMenuEnd;
  if (0 == strcmp(event, "menuListItemSelected"))
    return ax::Event::kMenuListItemSelected;
  if (0 == strcmp(event, "menuListValueChanged"))
    return ax::Event::kMenuListValueChanged;
  if (0 == strcmp(event, "menuPopupEnd"))
    return ax::Event::kMenuPopupEnd;
  if (0 == strcmp(event, "menuPopupStart"))
    return ax::Event::kMenuPopupStart;
  if (0 == strcmp(event, "menuStart"))
    return ax::Event::kMenuStart;
  if (0 == strcmp(event, "mouseCanceled"))
    return ax::Event::kMouseCanceled;
  if (0 == strcmp(event, "mouseDragged"))
    return ax::Event::kMouseDragged;
  if (0 == strcmp(event, "mouseMoved"))
    return ax::Event::kMouseMoved;
  if (0 == strcmp(event, "mousePressed"))
    return ax::Event::kMousePressed;
  if (0 == strcmp(event, "mouseReleased"))
    return ax::Event::kMouseReleased;
  if (0 == strcmp(event, "rowCollapsed"))
    return ax::Event::kRowCollapsed;
  if (0 == strcmp(event, "rowCountChanged"))
    return ax::Event::kRowCountChanged;
  if (0 == strcmp(event, "rowExpanded"))
    return ax::Event::kRowExpanded;
  if (0 == strcmp(event, "scrollPositionChanged"))
    return ax::Event::kScrollPositionChanged;
  if (0 == strcmp(event, "scrolledToAnchor"))
    return ax::Event::kScrolledToAnchor;
  if (0 == strcmp(event, "selectedChildrenChanged"))
    return ax::Event::kSelectedChildrenChanged;
  if (0 == strcmp(event, "selection"))
    return ax::Event::kSelection;
  if (0 == strcmp(event, "selectionAdd"))
    return ax::Event::kSelectionAdd;
  if (0 == strcmp(event, "selectionRemove"))
    return ax::Event::kSelectionRemove;
  if (0 == strcmp(event, "show"))
    return ax::Event::kShow;
  if (0 == strcmp(event, "stateChanged"))
    return ax::Event::kStateChanged;
  if (0 == strcmp(event, "textChanged"))
    return ax::Event::kTextChanged;
  if (0 == strcmp(event, "textSelectionChanged"))
    return ax::Event::kTextSelectionChanged;
  if (0 == strcmp(event, "tooltipClosed"))
    return ax::Event::kTooltipClosed;
  if (0 == strcmp(event, "tooltipOpened"))
    return ax::Event::kTooltipOpened;
  if (0 == strcmp(event, "windowActivated"))
    return ax::Event::kWindowActivated;
  if (0 == strcmp(event, "windowDeactivated"))
    return ax::Event::kWindowDeactivated;
  if (0 == strcmp(event, "windowVisibilityChanged"))
    return ax::Event::kWindowVisibilityChanged;
  if (0 == strcmp(event, "treeChanged"))
    return ax::Event::kTreeChanged;
  if (0 == strcmp(event, "valueChanged"))
    return ax::Event::kValueChanged;
  return ax::Event::kNone;
}

const char* ToString(ax::Role role) {
  switch (role) {
    case ax::Role::kNone:
      return "none";
    case ax::Role::kAbbr:
      return "abbr";
    case ax::Role::kAlertDialog:
      return "alertDialog";
    case ax::Role::kAlert:
      return "alert";
    case ax::Role::kAnchor:
      return "anchor";
    case ax::Role::kApplication:
      return "application";
    case ax::Role::kArticle:
      return "article";
    case ax::Role::kAudio:
      return "audio";
    case ax::Role::kBanner:
      return "banner";
    case ax::Role::kBlockquote:
      return "blockquote";
    case ax::Role::kButton:
      return "button";
    case ax::Role::kCanvas:
      return "canvas";
    case ax::Role::kCaption:
      return "caption";
    case ax::Role::kCaret:
      return "caret";
    case ax::Role::kCell:
      return "cell";
    case ax::Role::kCheckBox:
      return "checkBox";
    case ax::Role::kClient:
      return "client";
    case ax::Role::kCode:
      return "code";
    case ax::Role::kColorWell:
      return "colorWell";
    case ax::Role::kColumnHeader:
      return "columnHeader";
    case ax::Role::kColumn:
      return "column";
    case ax::Role::kComboBoxGrouping:
      return "comboBoxGrouping";
    case ax::Role::kComboBoxMenuButton:
      return "comboBoxMenuButton";
    case ax::Role::kComment:
      return "comment";
    case ax::Role::kComplementary:
      return "complementary";
    case ax::Role::kContentDeletion:
      return "contentDeletion";
    case ax::Role::kContentInsertion:
      return "contentInsertion";
    case ax::Role::kContentInfo:
      return "contentInfo";
    case ax::Role::kDate:
      return "date";
    case ax::Role::kDateTime:
      return "dateTime";
    case ax::Role::kDefinition:
      return "definition";
    case ax::Role::kDescriptionListDetail:
      return "descriptionListDetail";
    case ax::Role::kDescriptionList:
      return "descriptionList";
    case ax::Role::kDescriptionListTerm:
      return "descriptionListTerm";
    case ax::Role::kDesktop:
      return "desktop";
    case ax::Role::kDetails:
      return "details";
    case ax::Role::kDialog:
      return "dialog";
    case ax::Role::kDirectory:
      return "directory";
    case ax::Role::kDisclosureTriangle:
      return "disclosureTriangle";
    case ax::Role::kDocAbstract:
      return "docAbstract";
    case ax::Role::kDocAcknowledgments:
      return "docAcknowledgments";
    case ax::Role::kDocAfterword:
      return "docAfterword";
    case ax::Role::kDocAppendix:
      return "docAppendix";
    case ax::Role::kDocBackLink:
      return "docBackLink";
    case ax::Role::kDocBiblioEntry:
      return "docBiblioEntry";
    case ax::Role::kDocBibliography:
      return "docBibliography";
    case ax::Role::kDocBiblioRef:
      return "docBiblioRef";
    case ax::Role::kDocChapter:
      return "docChapter";
    case ax::Role::kDocColophon:
      return "docColophon";
    case ax::Role::kDocConclusion:
      return "docConclusion";
    case ax::Role::kDocCover:
      return "docCover";
    case ax::Role::kDocCredit:
      return "docCredit";
    case ax::Role::kDocCredits:
      return "docCredits";
    case ax::Role::kDocDedication:
      return "docDedication";
    case ax::Role::kDocEndnote:
      return "docEndnote";
    case ax::Role::kDocEndnotes:
      return "docEndnotes";
    case ax::Role::kDocEpigraph:
      return "docEpigraph";
    case ax::Role::kDocEpilogue:
      return "docEpilogue";
    case ax::Role::kDocErrata:
      return "docErrata";
    case ax::Role::kDocExample:
      return "docExample";
    case ax::Role::kDocFootnote:
      return "docFootnote";
    case ax::Role::kDocForeword:
      return "docForeword";
    case ax::Role::kDocGlossary:
      return "docGlossary";
    case ax::Role::kDocGlossRef:
      return "docGlossref";
    case ax::Role::kDocIndex:
      return "docIndex";
    case ax::Role::kDocIntroduction:
      return "docIntroduction";
    case ax::Role::kDocNoteRef:
      return "docNoteRef";
    case ax::Role::kDocNotice:
      return "docNotice";
    case ax::Role::kDocPageBreak:
      return "docPageBreak";
    case ax::Role::kDocPageList:
      return "docPageList";
    case ax::Role::kDocPart:
      return "docPart";
    case ax::Role::kDocPreface:
      return "docPreface";
    case ax::Role::kDocPrologue:
      return "docPrologue";
    case ax::Role::kDocPullquote:
      return "docPullquote";
    case ax::Role::kDocQna:
      return "docQna";
    case ax::Role::kDocSubtitle:
      return "docSubtitle";
    case ax::Role::kDocTip:
      return "docTip";
    case ax::Role::kDocToc:
      return "docToc";
    case ax::Role::kDocument:
      return "document";
    case ax::Role::kEmbeddedObject:
      return "embeddedObject";
    case ax::Role::kEmphasis:
      return "emphasis";
    case ax::Role::kFeed:
      return "feed";
    case ax::Role::kFigcaption:
      return "figcaption";
    case ax::Role::kFigure:
      return "figure";
    case ax::Role::kFooter:
      return "footer";
    case ax::Role::kFooterAsNonLandmark:
      return "footerAsNonLandmark";
    case ax::Role::kForm:
      return "form";
    case ax::Role::kGenericContainer:
      return "genericContainer";
    case ax::Role::kGraphicsDocument:
      return "graphicsDocument";
    case ax::Role::kGraphicsObject:
      return "graphicsObject";
    case ax::Role::kGraphicsSymbol:
      return "graphicsSymbol";
    case ax::Role::kGrid:
      return "grid";
    case ax::Role::kGroup:
      return "group";
    case ax::Role::kHeader:
      return "header";
    case ax::Role::kHeaderAsNonLandmark:
      return "headerAsNonLandmark";
    case ax::Role::kHeading:
      return "heading";
    case ax::Role::kIframe:
      return "iframe";
    case ax::Role::kIframePresentational:
      return "iframePresentational";
    case ax::Role::kIgnored:
      return "ignored";
    case ax::Role::kImageMap:
      return "imageMap";
    case ax::Role::kImage:
      return "image";
    case ax::Role::kImeCandidate:
      return "imeCandidate";
    case ax::Role::kInlineTextBox:
      return "inlineTextBox";
    case ax::Role::kInputTime:
      return "inputTime";
    case ax::Role::kKeyboard:
      return "keyboard";
    case ax::Role::kLabelText:
      return "labelText";
    case ax::Role::kLayoutTable:
      return "layoutTable";
    case ax::Role::kLayoutTableCell:
      return "layoutTableCell";
    case ax::Role::kLayoutTableRow:
      return "layoutTableRow";
    case ax::Role::kLegend:
      return "legend";
    case ax::Role::kLineBreak:
      return "lineBreak";
    case ax::Role::kLink:
      return "link";
    case ax::Role::kList:
      return "list";
    case ax::Role::kListBoxOption:
      return "listBoxOption";
    case ax::Role::kListBox:
      return "listBox";
    case ax::Role::kListGrid:
      return "listGrid";
    case ax::Role::kListItem:
      return "listItem";
    case ax::Role::kListMarker:
      return "listMarker";
    case ax::Role::kLog:
      return "log";
    case ax::Role::kMain:
      return "main";
    case ax::Role::kMark:
      return "mark";
    case ax::Role::kMarquee:
      return "marquee";
    case ax::Role::kMath:
      return "math";
    case ax::Role::kMenu:
      return "menu";
    case ax::Role::kMenuBar:
      return "menuBar";
    case ax::Role::kMenuItem:
      return "menuItem";
    case ax::Role::kMenuItemCheckBox:
      return "menuItemCheckBox";
    case ax::Role::kMenuItemRadio:
      return "menuItemRadio";
    case ax::Role::kMenuListOption:
      return "menuListOption";
    case ax::Role::kMenuListPopup:
      return "menuListPopup";
    case ax::Role::kMeter:
      return "meter";
    case ax::Role::kNavigation:
      return "navigation";
    case ax::Role::kNote:
      return "note";
    case ax::Role::kPane:
      return "pane";
    case ax::Role::kParagraph:
      return "paragraph";
    case ax::Role::kPdfActionableHighlight:
      return "pdfActionableHighlight";
    case ax::Role::kPluginObject:
      return "pluginObject";
    case ax::Role::kPopUpButton:
      return "popUpButton";
    case ax::Role::kPortal:
      return "portal";
    case ax::Role::kPre:
      return "pre";
    case ax::Role::kPresentational:
      return "presentational";
    case ax::Role::kProgressIndicator:
      return "progressIndicator";
    case ax::Role::kRadioButton:
      return "radioButton";
    case ax::Role::kRadioGroup:
      return "radioGroup";
    case ax::Role::kRegion:
      return "region";
    case ax::Role::kRootWebArea:
      return "rootWebArea";
    case ax::Role::kRow:
      return "row";
    case ax::Role::kRowGroup:
      return "rowGroup";
    case ax::Role::kRowHeader:
      return "rowHeader";
    case ax::Role::kRuby:
      return "ruby";
    case ax::Role::kRubyAnnotation:
      return "rubyAnnotation";
    case ax::Role::kSection:
      return "section";
    case ax::Role::kStrong:
      return "strong";
    case ax::Role::kSuggestion:
      return "suggestion";
    case ax::Role::kSvgRoot:
      return "svgRoot";
    case ax::Role::kScrollBar:
      return "scrollBar";
    case ax::Role::kScrollView:
      return "scrollView";
    case ax::Role::kSearch:
      return "search";
    case ax::Role::kSearchBox:
      return "searchBox";
    case ax::Role::kSlider:
      return "slider";
    case ax::Role::kSliderThumb:
      return "sliderThumb";
    case ax::Role::kSpinButton:
      return "spinButton";
    case ax::Role::kSplitter:
      return "splitter";
    case ax::Role::kStaticText:
      return "staticText";
    case ax::Role::kStatus:
      return "status";
    case ax::Role::kSwitch:
      return "switch";
    case ax::Role::kTabList:
      return "tabList";
    case ax::Role::kTabPanel:
      return "tabPanel";
    case ax::Role::kTab:
      return "tab";
    case ax::Role::kTable:
      return "table";
    case ax::Role::kTableHeaderContainer:
      return "tableHeaderContainer";
    case ax::Role::kTerm:
      return "term";
    case ax::Role::kTextField:
      return "textField";
    case ax::Role::kTextFieldWithComboBox:
      return "textFieldWithComboBox";
    case ax::Role::kTime:
      return "time";
    case ax::Role::kTimer:
      return "timer";
    case ax::Role::kTitleBar:
      return "titleBar";
    case ax::Role::kToggleButton:
      return "toggleButton";
    case ax::Role::kToolbar:
      return "toolbar";
    case ax::Role::kTreeGrid:
      return "treeGrid";
    case ax::Role::kTreeItem:
      return "treeItem";
    case ax::Role::kTree:
      return "tree";
    case ax::Role::kUnknown:
      return "unknown";
    case ax::Role::kTooltip:
      return "tooltip";
    case ax::Role::kVideo:
      return "video";
    case ax::Role::kWebArea:
      return "webArea";
    case ax::Role::kWebView:
      return "webView";
    case ax::Role::kWindow:
      return "window";
  }

  return "";
}

ax::Role ParseRole(const char* role) {
  if (0 == strcmp(role, "none"))
    return ax::Role::kNone;
  if (0 == strcmp(role, "abbr"))
    return ax::Role::kAbbr;
  if (0 == strcmp(role, "alertDialog"))
    return ax::Role::kAlertDialog;
  if (0 == strcmp(role, "alert"))
    return ax::Role::kAlert;
  if (0 == strcmp(role, "anchor"))
    return ax::Role::kAnchor;
  if (0 == strcmp(role, "application"))
    return ax::Role::kApplication;
  if (0 == strcmp(role, "article"))
    return ax::Role::kArticle;
  if (0 == strcmp(role, "audio"))
    return ax::Role::kAudio;
  if (0 == strcmp(role, "banner"))
    return ax::Role::kBanner;
  if (0 == strcmp(role, "blockquote"))
    return ax::Role::kBlockquote;
  if (0 == strcmp(role, "button"))
    return ax::Role::kButton;
  if (0 == strcmp(role, "canvas"))
    return ax::Role::kCanvas;
  if (0 == strcmp(role, "caption"))
    return ax::Role::kCaption;
  if (0 == strcmp(role, "caret"))
    return ax::Role::kCaret;
  if (0 == strcmp(role, "cell"))
    return ax::Role::kCell;
  if (0 == strcmp(role, "checkBox"))
    return ax::Role::kCheckBox;
  if (0 == strcmp(role, "client"))
    return ax::Role::kClient;
  if (0 == strcmp(role, "code"))
    return ax::Role::kCode;
  if (0 == strcmp(role, "colorWell"))
    return ax::Role::kColorWell;
  if (0 == strcmp(role, "columnHeader"))
    return ax::Role::kColumnHeader;
  if (0 == strcmp(role, "column"))
    return ax::Role::kColumn;
  if (0 == strcmp(role, "comboBoxGrouping"))
    return ax::Role::kComboBoxGrouping;
  if (0 == strcmp(role, "comboBoxMenuButton"))
    return ax::Role::kComboBoxMenuButton;
  if (0 == strcmp(role, "comment"))
    return ax::Role::kComment;
  if (0 == strcmp(role, "complementary"))
    return ax::Role::kComplementary;
  if (0 == strcmp(role, "contentDeletion"))
    return ax::Role::kContentDeletion;
  if (0 == strcmp(role, "contentInsertion"))
    return ax::Role::kContentInsertion;
  if (0 == strcmp(role, "contentInfo"))
    return ax::Role::kContentInfo;
  if (0 == strcmp(role, "date"))
    return ax::Role::kDate;
  if (0 == strcmp(role, "dateTime"))
    return ax::Role::kDateTime;
  if (0 == strcmp(role, "definition"))
    return ax::Role::kDefinition;
  if (0 == strcmp(role, "descriptionListDetail"))
    return ax::Role::kDescriptionListDetail;
  if (0 == strcmp(role, "descriptionList"))
    return ax::Role::kDescriptionList;
  if (0 == strcmp(role, "descriptionListTerm"))
    return ax::Role::kDescriptionListTerm;
  if (0 == strcmp(role, "desktop"))
    return ax::Role::kDesktop;
  if (0 == strcmp(role, "details"))
    return ax::Role::kDetails;
  if (0 == strcmp(role, "dialog"))
    return ax::Role::kDialog;
  if (0 == strcmp(role, "directory"))
    return ax::Role::kDirectory;
  if (0 == strcmp(role, "disclosureTriangle"))
    return ax::Role::kDisclosureTriangle;
  if (0 == strcmp(role, "docAbstract"))
    return ax::Role::kDocAbstract;
  if (0 == strcmp(role, "docAcknowledgments"))
    return ax::Role::kDocAcknowledgments;
  if (0 == strcmp(role, "docAfterword"))
    return ax::Role::kDocAfterword;
  if (0 == strcmp(role, "docAppendix"))
    return ax::Role::kDocAppendix;
  if (0 == strcmp(role, "docBackLink"))
    return ax::Role::kDocBackLink;
  if (0 == strcmp(role, "docBiblioEntry"))
    return ax::Role::kDocBiblioEntry;
  if (0 == strcmp(role, "docBibliography"))
    return ax::Role::kDocBibliography;
  if (0 == strcmp(role, "docBiblioRef"))
    return ax::Role::kDocBiblioRef;
  if (0 == strcmp(role, "docChapter"))
    return ax::Role::kDocChapter;
  if (0 == strcmp(role, "docColophon"))
    return ax::Role::kDocColophon;
  if (0 == strcmp(role, "docConclusion"))
    return ax::Role::kDocConclusion;
  if (0 == strcmp(role, "docCover"))
    return ax::Role::kDocCover;
  if (0 == strcmp(role, "docCredit"))
    return ax::Role::kDocCredit;
  if (0 == strcmp(role, "docCredits"))
    return ax::Role::kDocCredits;
  if (0 == strcmp(role, "docDedication"))
    return ax::Role::kDocDedication;
  if (0 == strcmp(role, "docEndnote"))
    return ax::Role::kDocEndnote;
  if (0 == strcmp(role, "docEndnotes"))
    return ax::Role::kDocEndnotes;
  if (0 == strcmp(role, "docEpigraph"))
    return ax::Role::kDocEpigraph;
  if (0 == strcmp(role, "docEpilogue"))
    return ax::Role::kDocEpilogue;
  if (0 == strcmp(role, "docErrata"))
    return ax::Role::kDocErrata;
  if (0 == strcmp(role, "docExample"))
    return ax::Role::kDocExample;
  if (0 == strcmp(role, "docFootnote"))
    return ax::Role::kDocFootnote;
  if (0 == strcmp(role, "docForeword"))
    return ax::Role::kDocForeword;
  if (0 == strcmp(role, "docGlossary"))
    return ax::Role::kDocGlossary;
  if (0 == strcmp(role, "docGlossref"))
    return ax::Role::kDocGlossRef;
  if (0 == strcmp(role, "docIndex"))
    return ax::Role::kDocIndex;
  if (0 == strcmp(role, "docIntroduction"))
    return ax::Role::kDocIntroduction;
  if (0 == strcmp(role, "docNoteRef"))
    return ax::Role::kDocNoteRef;
  if (0 == strcmp(role, "docNotice"))
    return ax::Role::kDocNotice;
  if (0 == strcmp(role, "docPageBreak"))
    return ax::Role::kDocPageBreak;
  if (0 == strcmp(role, "docPageList"))
    return ax::Role::kDocPageList;
  if (0 == strcmp(role, "docPart"))
    return ax::Role::kDocPart;
  if (0 == strcmp(role, "docPreface"))
    return ax::Role::kDocPreface;
  if (0 == strcmp(role, "docPrologue"))
    return ax::Role::kDocPrologue;
  if (0 == strcmp(role, "docPullquote"))
    return ax::Role::kDocPullquote;
  if (0 == strcmp(role, "docQna"))
    return ax::Role::kDocQna;
  if (0 == strcmp(role, "docSubtitle"))
    return ax::Role::kDocSubtitle;
  if (0 == strcmp(role, "docTip"))
    return ax::Role::kDocTip;
  if (0 == strcmp(role, "docToc"))
    return ax::Role::kDocToc;
  if (0 == strcmp(role, "document"))
    return ax::Role::kDocument;
  if (0 == strcmp(role, "embeddedObject"))
    return ax::Role::kEmbeddedObject;
  if (0 == strcmp(role, "emphasis"))
    return ax::Role::kEmphasis;
  if (0 == strcmp(role, "feed"))
    return ax::Role::kFeed;
  if (0 == strcmp(role, "figcaption"))
    return ax::Role::kFigcaption;
  if (0 == strcmp(role, "figure"))
    return ax::Role::kFigure;
  if (0 == strcmp(role, "footer"))
    return ax::Role::kFooter;
  if (0 == strcmp(role, "footerAsNonLandmark"))
    return ax::Role::kFooterAsNonLandmark;
  if (0 == strcmp(role, "form"))
    return ax::Role::kForm;
  if (0 == strcmp(role, "genericContainer"))
    return ax::Role::kGenericContainer;
  if (0 == strcmp(role, "graphicsDocument"))
    return ax::Role::kGraphicsDocument;
  if (0 == strcmp(role, "graphicsObject"))
    return ax::Role::kGraphicsObject;
  if (0 == strcmp(role, "graphicsSymbol"))
    return ax::Role::kGraphicsSymbol;
  if (0 == strcmp(role, "grid"))
    return ax::Role::kGrid;
  if (0 == strcmp(role, "group"))
    return ax::Role::kGroup;
  if (0 == strcmp(role, "heading"))
    return ax::Role::kHeading;
  if (0 == strcmp(role, "header"))
    return ax::Role::kHeader;
  if (0 == strcmp(role, "headerAsNonLandmark"))
    return ax::Role::kHeaderAsNonLandmark;
  if (0 == strcmp(role, "pdfActionableHighlight"))
    return ax::Role::kPdfActionableHighlight;
  if (0 == strcmp(role, "iframe"))
    return ax::Role::kIframe;
  if (0 == strcmp(role, "iframePresentational"))
    return ax::Role::kIframePresentational;
  if (0 == strcmp(role, "ignored"))
    return ax::Role::kIgnored;
  if (0 == strcmp(role, "imageMap"))
    return ax::Role::kImageMap;
  if (0 == strcmp(role, "image"))
    return ax::Role::kImage;
  if (0 == strcmp(role, "imeCandidate"))
    return ax::Role::kImeCandidate;
  if (0 == strcmp(role, "inlineTextBox"))
    return ax::Role::kInlineTextBox;
  if (0 == strcmp(role, "inputTime"))
    return ax::Role::kInputTime;
  if (0 == strcmp(role, "keyboard"))
    return ax::Role::kKeyboard;
  if (0 == strcmp(role, "labelText"))
    return ax::Role::kLabelText;
  if (0 == strcmp(role, "layoutTable"))
    return ax::Role::kLayoutTable;
  if (0 == strcmp(role, "layoutTableCell"))
    return ax::Role::kLayoutTableCell;
  if (0 == strcmp(role, "layoutTableRow"))
    return ax::Role::kLayoutTableRow;
  if (0 == strcmp(role, "legend"))
    return ax::Role::kLegend;
  if (0 == strcmp(role, "lineBreak"))
    return ax::Role::kLineBreak;
  if (0 == strcmp(role, "link"))
    return ax::Role::kLink;
  if (0 == strcmp(role, "listBoxOption"))
    return ax::Role::kListBoxOption;
  if (0 == strcmp(role, "listBox"))
    return ax::Role::kListBox;
  if (0 == strcmp(role, "listGrid"))
    return ax::Role::kListGrid;
  if (0 == strcmp(role, "listItem"))
    return ax::Role::kListItem;
  if (0 == strcmp(role, "listMarker"))
    return ax::Role::kListMarker;
  if (0 == strcmp(role, "list"))
    return ax::Role::kList;
  if (0 == strcmp(role, "log"))
    return ax::Role::kLog;
  if (0 == strcmp(role, "main"))
    return ax::Role::kMain;
  if (0 == strcmp(role, "mark"))
    return ax::Role::kMark;
  if (0 == strcmp(role, "marquee"))
    return ax::Role::kMarquee;
  if (0 == strcmp(role, "math"))
    return ax::Role::kMath;
  if (0 == strcmp(role, "menu"))
    return ax::Role::kMenu;
  if (0 == strcmp(role, "menuBar"))
    return ax::Role::kMenuBar;
  if (0 == strcmp(role, "menuItem"))
    return ax::Role::kMenuItem;
  if (0 == strcmp(role, "menuItemCheckBox"))
    return ax::Role::kMenuItemCheckBox;
  if (0 == strcmp(role, "menuItemRadio"))
    return ax::Role::kMenuItemRadio;
  if (0 == strcmp(role, "menuListOption"))
    return ax::Role::kMenuListOption;
  if (0 == strcmp(role, "menuListPopup"))
    return ax::Role::kMenuListPopup;
  if (0 == strcmp(role, "meter"))
    return ax::Role::kMeter;
  if (0 == strcmp(role, "navigation"))
    return ax::Role::kNavigation;
  if (0 == strcmp(role, "note"))
    return ax::Role::kNote;
  if (0 == strcmp(role, "pane"))
    return ax::Role::kPane;
  if (0 == strcmp(role, "paragraph"))
    return ax::Role::kParagraph;
  if (0 == strcmp(role, "pluginObject"))
    return ax::Role::kPluginObject;
  if (0 == strcmp(role, "popUpButton"))
    return ax::Role::kPopUpButton;
  if (0 == strcmp(role, "portal"))
    return ax::Role::kPortal;
  if (0 == strcmp(role, "pre"))
    return ax::Role::kPre;
  if (0 == strcmp(role, "presentational"))
    return ax::Role::kPresentational;
  if (0 == strcmp(role, "progressIndicator"))
    return ax::Role::kProgressIndicator;
  if (0 == strcmp(role, "radioButton"))
    return ax::Role::kRadioButton;
  if (0 == strcmp(role, "radioGroup"))
    return ax::Role::kRadioGroup;
  if (0 == strcmp(role, "region"))
    return ax::Role::kRegion;
  if (0 == strcmp(role, "rootWebArea"))
    return ax::Role::kRootWebArea;
  if (0 == strcmp(role, "row"))
    return ax::Role::kRow;
  if (0 == strcmp(role, "rowGroup"))
    return ax::Role::kRowGroup;
  if (0 == strcmp(role, "rowHeader"))
    return ax::Role::kRowHeader;
  if (0 == strcmp(role, "ruby"))
    return ax::Role::kRuby;
  if (0 == strcmp(role, "rubyAnnotation"))
    return ax::Role::kRubyAnnotation;
  if (0 == strcmp(role, "section"))
    return ax::Role::kSection;
  if (0 == strcmp(role, "scrollBar"))
    return ax::Role::kScrollBar;
  if (0 == strcmp(role, "scrollView"))
    return ax::Role::kScrollView;
  if (0 == strcmp(role, "search"))
    return ax::Role::kSearch;
  if (0 == strcmp(role, "searchBox"))
    return ax::Role::kSearchBox;
  if (0 == strcmp(role, "slider"))
    return ax::Role::kSlider;
  if (0 == strcmp(role, "sliderThumb"))
    return ax::Role::kSliderThumb;
  if (0 == strcmp(role, "spinButton"))
    return ax::Role::kSpinButton;
  if (0 == strcmp(role, "splitter"))
    return ax::Role::kSplitter;
  if (0 == strcmp(role, "staticText"))
    return ax::Role::kStaticText;
  if (0 == strcmp(role, "status"))
    return ax::Role::kStatus;
  if (0 == strcmp(role, "suggestion"))
    return ax::Role::kSuggestion;
  if (0 == strcmp(role, "svgRoot"))
    return ax::Role::kSvgRoot;
  if (0 == strcmp(role, "switch"))
    return ax::Role::kSwitch;
  if (0 == strcmp(role, "strong"))
    return ax::Role::kStrong;
  if (0 == strcmp(role, "tabList"))
    return ax::Role::kTabList;
  if (0 == strcmp(role, "tabPanel"))
    return ax::Role::kTabPanel;
  if (0 == strcmp(role, "tab"))
    return ax::Role::kTab;
  if (0 == strcmp(role, "tableHeaderContainer"))
    return ax::Role::kTableHeaderContainer;
  if (0 == strcmp(role, "table"))
    return ax::Role::kTable;
  if (0 == strcmp(role, "term"))
    return ax::Role::kTerm;
  if (0 == strcmp(role, "textField"))
    return ax::Role::kTextField;
  if (0 == strcmp(role, "textFieldWithComboBox"))
    return ax::Role::kTextFieldWithComboBox;
  if (0 == strcmp(role, "time"))
    return ax::Role::kTime;
  if (0 == strcmp(role, "timer"))
    return ax::Role::kTimer;
  if (0 == strcmp(role, "titleBar"))
    return ax::Role::kTitleBar;
  if (0 == strcmp(role, "toggleButton"))
    return ax::Role::kToggleButton;
  if (0 == strcmp(role, "toolbar"))
    return ax::Role::kToolbar;
  if (0 == strcmp(role, "treeGrid"))
    return ax::Role::kTreeGrid;
  if (0 == strcmp(role, "treeItem"))
    return ax::Role::kTreeItem;
  if (0 == strcmp(role, "tree"))
    return ax::Role::kTree;
  if (0 == strcmp(role, "unknown"))
    return ax::Role::kUnknown;
  if (0 == strcmp(role, "tooltip"))
    return ax::Role::kTooltip;
  if (0 == strcmp(role, "video"))
    return ax::Role::kVideo;
  if (0 == strcmp(role, "webArea"))
    return ax::Role::kWebArea;
  if (0 == strcmp(role, "webView"))
    return ax::Role::kWebView;
  if (0 == strcmp(role, "window"))
    return ax::Role::kWindow;
  return ax::Role::kNone;
}

const char* ToString(ax::State state) {
  switch (state) {
    case ax::State::kNone:
      return "none";
    case ax::State::kAutofillAvailable:
      return "autofillAvailable";
    case ax::State::kCollapsed:
      return "collapsed";
    case ax::State::kDefault:
      return "default";
    case ax::State::kEditable:
      return "editable";
    case ax::State::kExpanded:
      return "expanded";
    case ax::State::kFocusable:
      return "focusable";
    case ax::State::kHorizontal:
      return "horizontal";
    case ax::State::kHovered:
      return "hovered";
    case ax::State::kIgnored:
      return "ignored";
    case ax::State::kInvisible:
      return "invisible";
    case ax::State::kLinked:
      return "linked";
    case ax::State::kMultiline:
      return "multiline";
    case ax::State::kMultiselectable:
      return "multiselectable";
    case ax::State::kProtected:
      return "protected";
    case ax::State::kRequired:
      return "required";
    case ax::State::kRichlyEditable:
      return "richlyEditable";
    case ax::State::kVertical:
      return "vertical";
    case ax::State::kVisited:
      return "visited";
    case ax::State::kMaxValue:
      return "visited";
  }

  return "";
}

ax::State ParseState(const char* state) {
  if (0 == strcmp(state, "none"))
    return ax::State::kNone;
  if (0 == strcmp(state, "autofillAvailable"))
    return ax::State::kAutofillAvailable;
  if (0 == strcmp(state, "collapsed"))
    return ax::State::kCollapsed;
  if (0 == strcmp(state, "default"))
    return ax::State::kDefault;
  if (0 == strcmp(state, "editable"))
    return ax::State::kEditable;
  if (0 == strcmp(state, "expanded"))
    return ax::State::kExpanded;
  if (0 == strcmp(state, "focusable"))
    return ax::State::kFocusable;
  if (0 == strcmp(state, "horizontal"))
    return ax::State::kHorizontal;
  if (0 == strcmp(state, "hovered"))
    return ax::State::kHovered;
  if (0 == strcmp(state, "ignored"))
    return ax::State::kIgnored;
  if (0 == strcmp(state, "invisible"))
    return ax::State::kInvisible;
  if (0 == strcmp(state, "linked"))
    return ax::State::kLinked;
  if (0 == strcmp(state, "multiline"))
    return ax::State::kMultiline;
  if (0 == strcmp(state, "multiselectable"))
    return ax::State::kMultiselectable;
  if (0 == strcmp(state, "protected"))
    return ax::State::kProtected;
  if (0 == strcmp(state, "required"))
    return ax::State::kRequired;
  if (0 == strcmp(state, "richlyEditable"))
    return ax::State::kRichlyEditable;
  if (0 == strcmp(state, "vertical"))
    return ax::State::kVertical;
  if (0 == strcmp(state, "visited"))
    return ax::State::kVisited;
  return ax::State::kNone;
}

const char* ToString(ax::Action action) {
  switch (action) {
    case ax::Action::kNone:
      return "none";
    case ax::Action::kBlur:
      return "blur";
    case ax::Action::kClearAccessibilityFocus:
      return "clearAccessibilityFocus";
    case ax::Action::kCollapse:
      return "collapse";
    case ax::Action::kCustomAction:
      return "customAction";
    case ax::Action::kDecrement:
      return "decrement";
    case ax::Action::kDoDefault:
      return "doDefault";
    case ax::Action::kExpand:
      return "expand";
    case ax::Action::kFocus:
      return "focus";
    case ax::Action::kGetImageData:
      return "getImageData";
    case ax::Action::kHitTest:
      return "hitTest";
    case ax::Action::kIncrement:
      return "increment";
    case ax::Action::kLoadInlineTextBoxes:
      return "loadInlineTextBoxes";
    case ax::Action::kReplaceSelectedText:
      return "replaceSelectedText";
    case ax::Action::kScrollBackward:
      return "scrollBackward";
    case ax::Action::kScrollForward:
      return "scrollForward";
    case ax::Action::kScrollUp:
      return "scrollUp";
    case ax::Action::kScrollDown:
      return "scrollDown";
    case ax::Action::kScrollLeft:
      return "scrollLeft";
    case ax::Action::kScrollRight:
      return "scrollRight";
    case ax::Action::kScrollToMakeVisible:
      return "scrollToMakeVisible";
    case ax::Action::kScrollToPoint:
      return "scrollToPoint";
    case ax::Action::kSetAccessibilityFocus:
      return "setAccessibilityFocus";
    case ax::Action::kSetScrollOffset:
      return "setScrollOffset";
    case ax::Action::kSetSelection:
      return "setSelection";
    case ax::Action::kSetSequentialFocusNavigationStartingPoint:
      return "setSequentialFocusNavigationStartingPoint";
    case ax::Action::kSetValue:
      return "setValue";
    case ax::Action::kShowContextMenu:
      return "showContextMenu";
    case ax::Action::kGetTextLocation:
      return "getTextLocation";
    case ax::Action::kAnnotatePageImages:
      return "annotatePageImages";
    case ax::Action::kSignalEndOfTest:
      return "signalEndOfTest";
    case ax::Action::kShowTooltip:
      return "showTooltip";
    case ax::Action::kHideTooltip:
      return "hideTooltip";
    case ax::Action::kInternalInvalidateTree:
      return "internalInvalidateTree";
    case ax::Action::kMaxValue:
      return "";
  }

  return "";
}

ax::Action ParseAction(const char* action) {
  if (0 == strcmp(action, "none"))
    return ax::Action::kNone;
  if (0 == strcmp(action, "annotatePageImages"))
    return ax::Action::kAnnotatePageImages;
  if (0 == strcmp(action, "blur"))
    return ax::Action::kBlur;
  if (0 == strcmp(action, "clearAccessibilityFocus"))
    return ax::Action::kClearAccessibilityFocus;
  if (0 == strcmp(action, "collapse"))
    return ax::Action::kCollapse;
  if (0 == strcmp(action, "customAction"))
    return ax::Action::kCustomAction;
  if (0 == strcmp(action, "decrement"))
    return ax::Action::kDecrement;
  if (0 == strcmp(action, "doDefault"))
    return ax::Action::kDoDefault;
  if (0 == strcmp(action, "expand"))
    return ax::Action::kExpand;
  if (0 == strcmp(action, "focus"))
    return ax::Action::kFocus;
  if (0 == strcmp(action, "getImageData"))
    return ax::Action::kGetImageData;
  if (0 == strcmp(action, "getTextLocation"))
    return ax::Action::kGetTextLocation;
  if (0 == strcmp(action, "hitTest"))
    return ax::Action::kHitTest;
  if (0 == strcmp(action, "increment"))
    return ax::Action::kIncrement;
  if (0 == strcmp(action, "loadInlineTextBoxes"))
    return ax::Action::kLoadInlineTextBoxes;
  if (0 == strcmp(action, "replaceSelectedText"))
    return ax::Action::kReplaceSelectedText;
  if (0 == strcmp(action, "scrollBackward"))
    return ax::Action::kScrollBackward;
  if (0 == strcmp(action, "scrollForward"))
    return ax::Action::kScrollForward;
  if (0 == strcmp(action, "scrollUp"))
    return ax::Action::kScrollUp;
  if (0 == strcmp(action, "scrollDown"))
    return ax::Action::kScrollDown;
  if (0 == strcmp(action, "scrollLeft"))
    return ax::Action::kScrollLeft;
  if (0 == strcmp(action, "scrollRight"))
    return ax::Action::kScrollRight;
  if (0 == strcmp(action, "scrollToMakeVisible"))
    return ax::Action::kScrollToMakeVisible;
  if (0 == strcmp(action, "scrollToPoint"))
    return ax::Action::kScrollToPoint;
  if (0 == strcmp(action, "setAccessibilityFocus"))
    return ax::Action::kSetAccessibilityFocus;
  if (0 == strcmp(action, "setScrollOffset"))
    return ax::Action::kSetScrollOffset;
  if (0 == strcmp(action, "setSelection"))
    return ax::Action::kSetSelection;
  if (0 == strcmp(action, "setSequentialFocusNavigationStartingPoint"))
    return ax::Action::kSetSequentialFocusNavigationStartingPoint;
  if (0 == strcmp(action, "setValue"))
    return ax::Action::kSetValue;
  if (0 == strcmp(action, "showContextMenu"))
    return ax::Action::kShowContextMenu;
  if (0 == strcmp(action, "signalEndOfTest"))
    return ax::Action::kSignalEndOfTest;
  if (0 == strcmp(action, "showTooltip"))
    return ax::Action::kShowTooltip;
  if (0 == strcmp(action, "hideTooltip"))
    return ax::Action::kHideTooltip;
  if (0 == strcmp(action, "internalInvalidateTree"))
    return ax::Action::kInternalInvalidateTree;
  return ax::Action::kNone;
}

const char* ToString(ax::ActionFlags action_flags) {
  switch (action_flags) {
    case ax::ActionFlags::kNone:
      return "none";
    case ax::ActionFlags::kRequestImages:
      return "requestImages";
    case ax::ActionFlags::kRequestInlineTextBoxes:
      return "requestInlineTextBoxes";
  }

  return "";
}

ax::ActionFlags ParseActionFlags(const char* action_flags) {
  if (0 == strcmp(action_flags, "none"))
    return ax::ActionFlags::kNone;
  if (0 == strcmp(action_flags, "requestImages"))
    return ax::ActionFlags::kRequestImages;
  if (0 == strcmp(action_flags, "requestInlineTextBoxes"))
    return ax::ActionFlags::kRequestInlineTextBoxes;
  return ax::ActionFlags::kNone;
}

const char* ToString(ax::ScrollAlignment scroll_alignment) {
  switch (scroll_alignment) {
    case ax::ScrollAlignment::kNone:
      return "none";
    case ax::ScrollAlignment::kScrollAlignmentCenter:
      return "scrollAlignmentCenter";
    case ax::ScrollAlignment::kScrollAlignmentTop:
      return "scrollAlignmentTop";
    case ax::ScrollAlignment::kScrollAlignmentBottom:
      return "scrollAlignmentBottom";
    case ax::ScrollAlignment::kScrollAlignmentLeft:
      return "scrollAlignmentLeft";
    case ax::ScrollAlignment::kScrollAlignmentRight:
      return "scrollAlignmentRight";
    case ax::ScrollAlignment::kScrollAlignmentClosestEdge:
      return "scrollAlignmentClosestEdge";
  }
}

ax::ScrollAlignment ParseScrollAlignment(const char* scroll_alignment) {
  if (0 == strcmp(scroll_alignment, "none"))
    return ax::ScrollAlignment::kNone;
  if (0 == strcmp(scroll_alignment, "scrollAlignmentCenter"))
    return ax::ScrollAlignment::kScrollAlignmentCenter;
  if (0 == strcmp(scroll_alignment, "scrollAlignmentTop"))
    return ax::ScrollAlignment::kScrollAlignmentTop;
  if (0 == strcmp(scroll_alignment, "scrollAlignmentBottom"))
    return ax::ScrollAlignment::kScrollAlignmentBottom;
  if (0 == strcmp(scroll_alignment, "scrollAlignmentLeft"))
    return ax::ScrollAlignment::kScrollAlignmentLeft;
  if (0 == strcmp(scroll_alignment, "scrollAlignmentRight"))
    return ax::ScrollAlignment::kScrollAlignmentRight;
  if (0 == strcmp(scroll_alignment, "scrollAlignmentClosestEdge"))
    return ax::ScrollAlignment::kScrollAlignmentClosestEdge;
  return ax::ScrollAlignment::kNone;
}

const char* ToString(ax::DefaultActionVerb default_action_verb) {
  switch (default_action_verb) {
    case ax::DefaultActionVerb::kNone:
      return "none";
    case ax::DefaultActionVerb::kActivate:
      return "activate";
    case ax::DefaultActionVerb::kCheck:
      return "check";
    case ax::DefaultActionVerb::kClick:
      return "click";
    case ax::DefaultActionVerb::kClickAncestor:
      // Some screen readers, such as Jaws, expect the following spelling of
      // this verb.
      return "click-ancestor";
    case ax::DefaultActionVerb::kJump:
      return "jump";
    case ax::DefaultActionVerb::kOpen:
      return "open";
    case ax::DefaultActionVerb::kPress:
      return "press";
    case ax::DefaultActionVerb::kSelect:
      return "select";
    case ax::DefaultActionVerb::kUncheck:
      return "uncheck";
  }

  return "";
}

ax::DefaultActionVerb ParseDefaultActionVerb(
    const char* default_action_verb) {
  if (0 == strcmp(default_action_verb, "none"))
    return ax::DefaultActionVerb::kNone;
  if (0 == strcmp(default_action_verb, "activate"))
    return ax::DefaultActionVerb::kActivate;
  if (0 == strcmp(default_action_verb, "check"))
    return ax::DefaultActionVerb::kCheck;
  if (0 == strcmp(default_action_verb, "click"))
    return ax::DefaultActionVerb::kClick;
  // Some screen readers, such as Jaws, expect the following spelling of this
  // verb.
  if (0 == strcmp(default_action_verb, "click-ancestor"))
    return ax::DefaultActionVerb::kClickAncestor;
  if (0 == strcmp(default_action_verb, "jump"))
    return ax::DefaultActionVerb::kJump;
  if (0 == strcmp(default_action_verb, "open"))
    return ax::DefaultActionVerb::kOpen;
  if (0 == strcmp(default_action_verb, "press"))
    return ax::DefaultActionVerb::kPress;
  if (0 == strcmp(default_action_verb, "select"))
    return ax::DefaultActionVerb::kSelect;
  if (0 == strcmp(default_action_verb, "uncheck"))
    return ax::DefaultActionVerb::kUncheck;
  return ax::DefaultActionVerb::kNone;
}

const char* ToString(ax::Mutation mutation) {
  switch (mutation) {
    case ax::Mutation::kNone:
      return "none";
    case ax::Mutation::kNodeCreated:
      return "nodeCreated";
    case ax::Mutation::kSubtreeCreated:
      return "subtreeCreated";
    case ax::Mutation::kNodeChanged:
      return "nodeChanged";
    case ax::Mutation::kNodeRemoved:
      return "nodeRemoved";
  }

  return "";
}

ax::Mutation ParseMutation(const char* mutation) {
  if (0 == strcmp(mutation, "none"))
    return ax::Mutation::kNone;
  if (0 == strcmp(mutation, "nodeCreated"))
    return ax::Mutation::kNodeCreated;
  if (0 == strcmp(mutation, "subtreeCreated"))
    return ax::Mutation::kSubtreeCreated;
  if (0 == strcmp(mutation, "nodeChanged"))
    return ax::Mutation::kNodeChanged;
  if (0 == strcmp(mutation, "nodeRemoved"))
    return ax::Mutation::kNodeRemoved;
  return ax::Mutation::kNone;
}

const char* ToString(ax::StringAttribute string_attribute) {
  switch (string_attribute) {
    case ax::StringAttribute::kNone:
      return "none";
    case ax::StringAttribute::kAccessKey:
      return "accessKey";
    case ax::StringAttribute::kAriaInvalidValue:
      return "ariaInvalidValue";
    case ax::StringAttribute::kAutoComplete:
      return "autoComplete";
    case ax::StringAttribute::kChildTreeId:
      return "childTreeId";
    case ax::StringAttribute::kClassName:
      return "className";
    case ax::StringAttribute::kContainerLiveRelevant:
      return "containerLiveRelevant";
    case ax::StringAttribute::kContainerLiveStatus:
      return "containerLiveStatus";
    case ax::StringAttribute::kDescription:
      return "description";
    case ax::StringAttribute::kDisplay:
      return "display";
    case ax::StringAttribute::kFontFamily:
      return "fontFamily";
    case ax::StringAttribute::kHtmlTag:
      return "htmlTag";
    case ax::StringAttribute::kImageAnnotation:
      return "imageAnnotation";
    case ax::StringAttribute::kImageDataUrl:
      return "imageDataUrl";
    case ax::StringAttribute::kInnerHtml:
      return "innerHtml";
    case ax::StringAttribute::kInputType:
      return "inputType";
    case ax::StringAttribute::kKeyShortcuts:
      return "keyShortcuts";
    case ax::StringAttribute::kLanguage:
      return "language";
    case ax::StringAttribute::kName:
      return "name";
    case ax::StringAttribute::kLiveRelevant:
      return "liveRelevant";
    case ax::StringAttribute::kLiveStatus:
      return "liveStatus";
    case ax::StringAttribute::kPlaceholder:
      return "placeholder";
    case ax::StringAttribute::kRole:
      return "role";
    case ax::StringAttribute::kRoleDescription:
      return "roleDescription";
    case ax::StringAttribute::kTooltip:
      return "tooltip";
    case ax::StringAttribute::kUrl:
      return "url";
    case ax::StringAttribute::kValue:
      return "value";
  }

  return "";
}

ax::StringAttribute ParseStringAttribute(const char* string_attribute) {
  if (0 == strcmp(string_attribute, "none"))
    return ax::StringAttribute::kNone;
  if (0 == strcmp(string_attribute, "accessKey"))
    return ax::StringAttribute::kAccessKey;
  if (0 == strcmp(string_attribute, "ariaInvalidValue"))
    return ax::StringAttribute::kAriaInvalidValue;
  if (0 == strcmp(string_attribute, "autoComplete"))
    return ax::StringAttribute::kAutoComplete;
  if (0 == strcmp(string_attribute, "childTreeId"))
    return ax::StringAttribute::kChildTreeId;
  if (0 == strcmp(string_attribute, "className"))
    return ax::StringAttribute::kClassName;
  if (0 == strcmp(string_attribute, "containerLiveRelevant"))
    return ax::StringAttribute::kContainerLiveRelevant;
  if (0 == strcmp(string_attribute, "containerLiveStatus"))
    return ax::StringAttribute::kContainerLiveStatus;
  if (0 == strcmp(string_attribute, "description"))
    return ax::StringAttribute::kDescription;
  if (0 == strcmp(string_attribute, "display"))
    return ax::StringAttribute::kDisplay;
  if (0 == strcmp(string_attribute, "fontFamily"))
    return ax::StringAttribute::kFontFamily;
  if (0 == strcmp(string_attribute, "htmlTag"))
    return ax::StringAttribute::kHtmlTag;
  if (0 == strcmp(string_attribute, "imageAnnotation"))
    return ax::StringAttribute::kImageAnnotation;
  if (0 == strcmp(string_attribute, "imageDataUrl"))
    return ax::StringAttribute::kImageDataUrl;
  if (0 == strcmp(string_attribute, "innerHtml"))
    return ax::StringAttribute::kInnerHtml;
  if (0 == strcmp(string_attribute, "inputType"))
    return ax::StringAttribute::kInputType;
  if (0 == strcmp(string_attribute, "keyShortcuts"))
    return ax::StringAttribute::kKeyShortcuts;
  if (0 == strcmp(string_attribute, "language"))
    return ax::StringAttribute::kLanguage;
  if (0 == strcmp(string_attribute, "name"))
    return ax::StringAttribute::kName;
  if (0 == strcmp(string_attribute, "liveRelevant"))
    return ax::StringAttribute::kLiveRelevant;
  if (0 == strcmp(string_attribute, "liveStatus"))
    return ax::StringAttribute::kLiveStatus;
  if (0 == strcmp(string_attribute, "placeholder"))
    return ax::StringAttribute::kPlaceholder;
  if (0 == strcmp(string_attribute, "role"))
    return ax::StringAttribute::kRole;
  if (0 == strcmp(string_attribute, "roleDescription"))
    return ax::StringAttribute::kRoleDescription;
  if (0 == strcmp(string_attribute, "tooltip"))
    return ax::StringAttribute::kTooltip;
  if (0 == strcmp(string_attribute, "url"))
    return ax::StringAttribute::kUrl;
  if (0 == strcmp(string_attribute, "value"))
    return ax::StringAttribute::kValue;
  return ax::StringAttribute::kNone;
}

const char* ToString(ax::IntAttribute int_attribute) {
  switch (int_attribute) {
    case ax::IntAttribute::kNone:
      return "none";
    case ax::IntAttribute::kDefaultActionVerb:
      return "defaultActionVerb";
    case ax::IntAttribute::kDropeffect:
      return "dropeffect";
    case ax::IntAttribute::kScrollX:
      return "scrollX";
    case ax::IntAttribute::kScrollXMin:
      return "scrollXMin";
    case ax::IntAttribute::kScrollXMax:
      return "scrollXMax";
    case ax::IntAttribute::kScrollY:
      return "scrollY";
    case ax::IntAttribute::kScrollYMin:
      return "scrollYMin";
    case ax::IntAttribute::kScrollYMax:
      return "scrollYMax";
    case ax::IntAttribute::kTextSelStart:
      return "textSelStart";
    case ax::IntAttribute::kTextSelEnd:
      return "textSelEnd";
    case ax::IntAttribute::kAriaColumnCount:
      return "ariaColumnCount";
    case ax::IntAttribute::kAriaCellColumnIndex:
      return "ariaCellColumnIndex";
    case ax::IntAttribute::kAriaCellColumnSpan:
      return "ariaCellColumnSpan";
    case ax::IntAttribute::kAriaRowCount:
      return "ariaRowCount";
    case ax::IntAttribute::kAriaCellRowIndex:
      return "ariaCellRowIndex";
    case ax::IntAttribute::kAriaCellRowSpan:
      return "ariaCellRowSpan";
    case ax::IntAttribute::kTableRowCount:
      return "tableRowCount";
    case ax::IntAttribute::kTableColumnCount:
      return "tableColumnCount";
    case ax::IntAttribute::kTableHeaderId:
      return "tableHeaderId";
    case ax::IntAttribute::kTableRowIndex:
      return "tableRowIndex";
    case ax::IntAttribute::kTableRowHeaderId:
      return "tableRowHeaderId";
    case ax::IntAttribute::kTableColumnIndex:
      return "tableColumnIndex";
    case ax::IntAttribute::kTableColumnHeaderId:
      return "tableColumnHeaderId";
    case ax::IntAttribute::kTableCellColumnIndex:
      return "tableCellColumnIndex";
    case ax::IntAttribute::kTableCellColumnSpan:
      return "tableCellColumnSpan";
    case ax::IntAttribute::kTableCellRowIndex:
      return "tableCellRowIndex";
    case ax::IntAttribute::kTableCellRowSpan:
      return "tableCellRowSpan";
    case ax::IntAttribute::kSortDirection:
      return "sortDirection";
    case ax::IntAttribute::kHierarchicalLevel:
      return "hierarchicalLevel";
    case ax::IntAttribute::kNameFrom:
      return "nameFrom";
    case ax::IntAttribute::kDescriptionFrom:
      return "descriptionFrom";
    case ax::IntAttribute::kActivedescendantId:
      return "activedescendantId";
    case ax::IntAttribute::kErrormessageId:
      return "errormessageId";
    case ax::IntAttribute::kInPageLinkTargetId:
      return "inPageLinkTargetId";
    case ax::IntAttribute::kMemberOfId:
      return "memberOfId";
    case ax::IntAttribute::kNextOnLineId:
      return "nextOnLineId";
    case ax::IntAttribute::kPopupForId:
      return "popupForId";
    case ax::IntAttribute::kPreviousOnLineId:
      return "previousOnLineId";
    case ax::IntAttribute::kRestriction:
      return "restriction";
    case ax::IntAttribute::kSetSize:
      return "setSize";
    case ax::IntAttribute::kPosInSet:
      return "posInSet";
    case ax::IntAttribute::kColorValue:
      return "colorValue";
    case ax::IntAttribute::kAriaCurrentState:
      return "ariaCurrentState";
    case ax::IntAttribute::kBackgroundColor:
      return "backgroundColor";
    case ax::IntAttribute::kColor:
      return "color";
    case ax::IntAttribute::kHasPopup:
      return "haspopup";
    case ax::IntAttribute::kInvalidState:
      return "invalidState";
    case ax::IntAttribute::kCheckedState:
      return "checkedState";
    case ax::IntAttribute::kListStyle:
      return "listStyle";
    case ax::IntAttribute::kTextAlign:
      return "text-align";
    case ax::IntAttribute::kTextDirection:
      return "textDirection";
    case ax::IntAttribute::kTextPosition:
      return "textPosition";
    case ax::IntAttribute::kTextStyle:
      return "textStyle";
    case ax::IntAttribute::kTextOverlineStyle:
      return "textOverlineStyle";
    case ax::IntAttribute::kTextStrikethroughStyle:
      return "textStrikethroughStyle";
    case ax::IntAttribute::kTextUnderlineStyle:
      return "textUnderlineStyle";
    case ax::IntAttribute::kPreviousFocusId:
      return "previousFocusId";
    case ax::IntAttribute::kNextFocusId:
      return "nextFocusId";
    case ax::IntAttribute::kImageAnnotationStatus:
      return "imageAnnotationStatus";
    case ax::IntAttribute::kDOMNodeId:
      return "domNodeId";
  }

  return "";
}

ax::IntAttribute ParseIntAttribute(const char* int_attribute) {
  if (0 == strcmp(int_attribute, "none"))
    return ax::IntAttribute::kNone;
  if (0 == strcmp(int_attribute, "defaultActionVerb"))
    return ax::IntAttribute::kDefaultActionVerb;
  if (0 == strcmp(int_attribute, "dropeffect"))
    return ax::IntAttribute::kDropeffect;
  if (0 == strcmp(int_attribute, "scrollX"))
    return ax::IntAttribute::kScrollX;
  if (0 == strcmp(int_attribute, "scrollXMin"))
    return ax::IntAttribute::kScrollXMin;
  if (0 == strcmp(int_attribute, "scrollXMax"))
    return ax::IntAttribute::kScrollXMax;
  if (0 == strcmp(int_attribute, "scrollY"))
    return ax::IntAttribute::kScrollY;
  if (0 == strcmp(int_attribute, "scrollYMin"))
    return ax::IntAttribute::kScrollYMin;
  if (0 == strcmp(int_attribute, "scrollYMax"))
    return ax::IntAttribute::kScrollYMax;
  if (0 == strcmp(int_attribute, "textSelStart"))
    return ax::IntAttribute::kTextSelStart;
  if (0 == strcmp(int_attribute, "textSelEnd"))
    return ax::IntAttribute::kTextSelEnd;
  if (0 == strcmp(int_attribute, "ariaColumnCount"))
    return ax::IntAttribute::kAriaColumnCount;
  if (0 == strcmp(int_attribute, "ariaCellColumnIndex"))
    return ax::IntAttribute::kAriaCellColumnIndex;
  if (0 == strcmp(int_attribute, "ariaCellColumnSpan"))
    return ax::IntAttribute::kAriaCellColumnSpan;
  if (0 == strcmp(int_attribute, "ariaRowCount"))
    return ax::IntAttribute::kAriaRowCount;
  if (0 == strcmp(int_attribute, "ariaCellRowIndex"))
    return ax::IntAttribute::kAriaCellRowIndex;
  if (0 == strcmp(int_attribute, "ariaCellRowSpan"))
    return ax::IntAttribute::kAriaCellRowSpan;
  if (0 == strcmp(int_attribute, "tableRowCount"))
    return ax::IntAttribute::kTableRowCount;
  if (0 == strcmp(int_attribute, "tableColumnCount"))
    return ax::IntAttribute::kTableColumnCount;
  if (0 == strcmp(int_attribute, "tableHeaderId"))
    return ax::IntAttribute::kTableHeaderId;
  if (0 == strcmp(int_attribute, "tableRowIndex"))
    return ax::IntAttribute::kTableRowIndex;
  if (0 == strcmp(int_attribute, "tableRowHeaderId"))
    return ax::IntAttribute::kTableRowHeaderId;
  if (0 == strcmp(int_attribute, "tableColumnIndex"))
    return ax::IntAttribute::kTableColumnIndex;
  if (0 == strcmp(int_attribute, "tableColumnHeaderId"))
    return ax::IntAttribute::kTableColumnHeaderId;
  if (0 == strcmp(int_attribute, "tableCellColumnIndex"))
    return ax::IntAttribute::kTableCellColumnIndex;
  if (0 == strcmp(int_attribute, "tableCellColumnSpan"))
    return ax::IntAttribute::kTableCellColumnSpan;
  if (0 == strcmp(int_attribute, "tableCellRowIndex"))
    return ax::IntAttribute::kTableCellRowIndex;
  if (0 == strcmp(int_attribute, "tableCellRowSpan"))
    return ax::IntAttribute::kTableCellRowSpan;
  if (0 == strcmp(int_attribute, "sortDirection"))
    return ax::IntAttribute::kSortDirection;
  if (0 == strcmp(int_attribute, "hierarchicalLevel"))
    return ax::IntAttribute::kHierarchicalLevel;
  if (0 == strcmp(int_attribute, "nameFrom"))
    return ax::IntAttribute::kNameFrom;
  if (0 == strcmp(int_attribute, "descriptionFrom"))
    return ax::IntAttribute::kDescriptionFrom;
  if (0 == strcmp(int_attribute, "activedescendantId"))
    return ax::IntAttribute::kActivedescendantId;
  if (0 == strcmp(int_attribute, "errormessageId"))
    return ax::IntAttribute::kErrormessageId;
  if (0 == strcmp(int_attribute, "inPageLinkTargetId"))
    return ax::IntAttribute::kInPageLinkTargetId;
  if (0 == strcmp(int_attribute, "memberOfId"))
    return ax::IntAttribute::kMemberOfId;
  if (0 == strcmp(int_attribute, "nextOnLineId"))
    return ax::IntAttribute::kNextOnLineId;
  if (0 == strcmp(int_attribute, "popupForId"))
    return ax::IntAttribute::kPopupForId;
  if (0 == strcmp(int_attribute, "previousOnLineId"))
    return ax::IntAttribute::kPreviousOnLineId;
  if (0 == strcmp(int_attribute, "restriction"))
    return ax::IntAttribute::kRestriction;
  if (0 == strcmp(int_attribute, "setSize"))
    return ax::IntAttribute::kSetSize;
  if (0 == strcmp(int_attribute, "posInSet"))
    return ax::IntAttribute::kPosInSet;
  if (0 == strcmp(int_attribute, "colorValue"))
    return ax::IntAttribute::kColorValue;
  if (0 == strcmp(int_attribute, "ariaCurrentState"))
    return ax::IntAttribute::kAriaCurrentState;
  if (0 == strcmp(int_attribute, "backgroundColor"))
    return ax::IntAttribute::kBackgroundColor;
  if (0 == strcmp(int_attribute, "color"))
    return ax::IntAttribute::kColor;
  if (0 == strcmp(int_attribute, "haspopup"))
    return ax::IntAttribute::kHasPopup;
  if (0 == strcmp(int_attribute, "invalidState"))
    return ax::IntAttribute::kInvalidState;
  if (0 == strcmp(int_attribute, "checkedState"))
    return ax::IntAttribute::kCheckedState;
  if (0 == strcmp(int_attribute, "listStyle"))
    return ax::IntAttribute::kListStyle;
  if (0 == strcmp(int_attribute, "text-align"))
    return ax::IntAttribute::kTextAlign;
  if (0 == strcmp(int_attribute, "textDirection"))
    return ax::IntAttribute::kTextDirection;
  if (0 == strcmp(int_attribute, "textPosition"))
    return ax::IntAttribute::kTextPosition;
  if (0 == strcmp(int_attribute, "textStyle"))
    return ax::IntAttribute::kTextStyle;
  if (0 == strcmp(int_attribute, "textOverlineStyle"))
    return ax::IntAttribute::kTextOverlineStyle;
  if (0 == strcmp(int_attribute, "textStrikethroughStyle"))
    return ax::IntAttribute::kTextStrikethroughStyle;
  if (0 == strcmp(int_attribute, "textUnderlineStyle"))
    return ax::IntAttribute::kTextUnderlineStyle;
  if (0 == strcmp(int_attribute, "previousFocusId"))
    return ax::IntAttribute::kPreviousFocusId;
  if (0 == strcmp(int_attribute, "nextFocusId"))
    return ax::IntAttribute::kNextFocusId;
  if (0 == strcmp(int_attribute, "imageAnnotationStatus"))
    return ax::IntAttribute::kImageAnnotationStatus;
  if (0 == strcmp(int_attribute, "domNodeId"))
    return ax::IntAttribute::kDOMNodeId;
  return ax::IntAttribute::kNone;
}

const char* ToString(ax::FloatAttribute float_attribute) {
  switch (float_attribute) {
    case ax::FloatAttribute::kNone:
      return "none";
    case ax::FloatAttribute::kValueForRange:
      return "valueForRange";
    case ax::FloatAttribute::kMinValueForRange:
      return "minValueForRange";
    case ax::FloatAttribute::kMaxValueForRange:
      return "maxValueForRange";
    case ax::FloatAttribute::kStepValueForRange:
      return "stepValueForRange";
    case ax::FloatAttribute::kFontSize:
      return "fontSize";
    case ax::FloatAttribute::kFontWeight:
      return "fontWeight";
    case ax::FloatAttribute::kTextIndent:
      return "textIndent";
  }

  return "";
}

ax::FloatAttribute ParseFloatAttribute(const char* float_attribute) {
  if (0 == strcmp(float_attribute, "none"))
    return ax::FloatAttribute::kNone;
  if (0 == strcmp(float_attribute, "valueForRange"))
    return ax::FloatAttribute::kValueForRange;
  if (0 == strcmp(float_attribute, "minValueForRange"))
    return ax::FloatAttribute::kMinValueForRange;
  if (0 == strcmp(float_attribute, "maxValueForRange"))
    return ax::FloatAttribute::kMaxValueForRange;
  if (0 == strcmp(float_attribute, "stepValueForRange"))
    return ax::FloatAttribute::kStepValueForRange;
  if (0 == strcmp(float_attribute, "fontSize"))
    return ax::FloatAttribute::kFontSize;
  if (0 == strcmp(float_attribute, "fontWeight"))
    return ax::FloatAttribute::kFontWeight;
  if (0 == strcmp(float_attribute, "textIndent"))
    return ax::FloatAttribute::kTextIndent;
  return ax::FloatAttribute::kNone;
}

const char* ToString(ax::BoolAttribute bool_attribute) {
  switch (bool_attribute) {
    case ax::BoolAttribute::kNone:
      return "none";
    case ax::BoolAttribute::kBusy:
      return "busy";
    case ax::BoolAttribute::kEditableRoot:
      return "editableRoot";
    case ax::BoolAttribute::kContainerLiveAtomic:
      return "containerLiveAtomic";
    case ax::BoolAttribute::kContainerLiveBusy:
      return "containerLiveBusy";
    case ax::BoolAttribute::kGrabbed:
      return "grabbed";
    case ax::BoolAttribute::kLiveAtomic:
      return "liveAtomic";
    case ax::BoolAttribute::kModal:
      return "modal";
    case ax::BoolAttribute::kUpdateLocationOnly:
      return "updateLocationOnly";
    case ax::BoolAttribute::kCanvasHasFallback:
      return "canvasHasFallback";
    case ax::BoolAttribute::kScrollable:
      return "scrollable";
    case ax::BoolAttribute::kClickable:
      return "clickable";
    case ax::BoolAttribute::kClipsChildren:
      return "clipsChildren";
    case ax::BoolAttribute::kNotUserSelectableStyle:
      return "notUserSelectableStyle";
    case ax::BoolAttribute::kSelected:
      return "selected";
    case ax::BoolAttribute::kSelectedFromFocus:
      return "selectedFromFocus";
    case ax::BoolAttribute::kSupportsTextLocation:
      return "supportsTextLocation";
    case ax::BoolAttribute::kIsLineBreakingObject:
      return "isLineBreakingObject";
    case ax::BoolAttribute::kIsPageBreakingObject:
      return "isPageBreakingObject";
    case ax::BoolAttribute::kHasAriaAttribute:
      return "hasAriaAttribute";
  }

  return "";
}

ax::BoolAttribute ParseBoolAttribute(const char* bool_attribute) {
  if (0 == strcmp(bool_attribute, "none"))
    return ax::BoolAttribute::kNone;
  if (0 == strcmp(bool_attribute, "busy"))
    return ax::BoolAttribute::kBusy;
  if (0 == strcmp(bool_attribute, "editableRoot"))
    return ax::BoolAttribute::kEditableRoot;
  if (0 == strcmp(bool_attribute, "containerLiveAtomic"))
    return ax::BoolAttribute::kContainerLiveAtomic;
  if (0 == strcmp(bool_attribute, "containerLiveBusy"))
    return ax::BoolAttribute::kContainerLiveBusy;
  if (0 == strcmp(bool_attribute, "grabbed"))
    return ax::BoolAttribute::kGrabbed;
  if (0 == strcmp(bool_attribute, "liveAtomic"))
    return ax::BoolAttribute::kLiveAtomic;
  if (0 == strcmp(bool_attribute, "modal"))
    return ax::BoolAttribute::kModal;
  if (0 == strcmp(bool_attribute, "updateLocationOnly"))
    return ax::BoolAttribute::kUpdateLocationOnly;
  if (0 == strcmp(bool_attribute, "canvasHasFallback"))
    return ax::BoolAttribute::kCanvasHasFallback;
  if (0 == strcmp(bool_attribute, "scrollable"))
    return ax::BoolAttribute::kScrollable;
  if (0 == strcmp(bool_attribute, "clickable"))
    return ax::BoolAttribute::kClickable;
  if (0 == strcmp(bool_attribute, "clipsChildren"))
    return ax::BoolAttribute::kClipsChildren;
  if (0 == strcmp(bool_attribute, "notUserSelectableStyle"))
    return ax::BoolAttribute::kNotUserSelectableStyle;
  if (0 == strcmp(bool_attribute, "selected"))
    return ax::BoolAttribute::kSelected;
  if (0 == strcmp(bool_attribute, "selectedFromFocus"))
    return ax::BoolAttribute::kSelectedFromFocus;
  if (0 == strcmp(bool_attribute, "supportsTextLocation"))
    return ax::BoolAttribute::kSupportsTextLocation;
  if (0 == strcmp(bool_attribute, "isLineBreakingObject"))
    return ax::BoolAttribute::kIsLineBreakingObject;
  if (0 == strcmp(bool_attribute, "isPageBreakingObject"))
    return ax::BoolAttribute::kIsPageBreakingObject;
  if (0 == strcmp(bool_attribute, "hasAriaAttribute"))
    return ax::BoolAttribute::kHasAriaAttribute;
  return ax::BoolAttribute::kNone;
}

const char* ToString(ax::IntListAttribute int_list_attribute) {
  switch (int_list_attribute) {
    case ax::IntListAttribute::kNone:
      return "none";
    case ax::IntListAttribute::kIndirectChildIds:
      return "indirectChildIds";
    case ax::IntListAttribute::kControlsIds:
      return "controlsIds";
    case ax::IntListAttribute::kDetailsIds:
      return "detailsIds";
    case ax::IntListAttribute::kDescribedbyIds:
      return "describedbyIds";
    case ax::IntListAttribute::kFlowtoIds:
      return "flowtoIds";
    case ax::IntListAttribute::kLabelledbyIds:
      return "labelledbyIds";
    case ax::IntListAttribute::kRadioGroupIds:
      return "radioGroupIds";
    case ax::IntListAttribute::kMarkerTypes:
      return "markerTypes";
    case ax::IntListAttribute::kMarkerStarts:
      return "markerStarts";
    case ax::IntListAttribute::kMarkerEnds:
      return "markerEnds";
    case ax::IntListAttribute::kCharacterOffsets:
      return "characterOffsets";
    case ax::IntListAttribute::kCachedLineStarts:
      return "cachedLineStarts";
    case ax::IntListAttribute::kWordStarts:
      return "wordStarts";
    case ax::IntListAttribute::kWordEnds:
      return "wordEnds";
    case ax::IntListAttribute::kCustomActionIds:
      return "customActionIds";
  }

  return "";
}

ax::IntListAttribute ParseIntListAttribute(
    const char* int_list_attribute) {
  if (0 == strcmp(int_list_attribute, "none"))
    return ax::IntListAttribute::kNone;
  if (0 == strcmp(int_list_attribute, "indirectChildIds"))
    return ax::IntListAttribute::kIndirectChildIds;
  if (0 == strcmp(int_list_attribute, "controlsIds"))
    return ax::IntListAttribute::kControlsIds;
  if (0 == strcmp(int_list_attribute, "detailsIds"))
    return ax::IntListAttribute::kDetailsIds;
  if (0 == strcmp(int_list_attribute, "describedbyIds"))
    return ax::IntListAttribute::kDescribedbyIds;
  if (0 == strcmp(int_list_attribute, "flowtoIds"))
    return ax::IntListAttribute::kFlowtoIds;
  if (0 == strcmp(int_list_attribute, "labelledbyIds"))
    return ax::IntListAttribute::kLabelledbyIds;
  if (0 == strcmp(int_list_attribute, "radioGroupIds"))
    return ax::IntListAttribute::kRadioGroupIds;
  if (0 == strcmp(int_list_attribute, "markerTypes"))
    return ax::IntListAttribute::kMarkerTypes;
  if (0 == strcmp(int_list_attribute, "markerStarts"))
    return ax::IntListAttribute::kMarkerStarts;
  if (0 == strcmp(int_list_attribute, "markerEnds"))
    return ax::IntListAttribute::kMarkerEnds;
  if (0 == strcmp(int_list_attribute, "characterOffsets"))
    return ax::IntListAttribute::kCharacterOffsets;
  if (0 == strcmp(int_list_attribute, "cachedLineStarts"))
    return ax::IntListAttribute::kCachedLineStarts;
  if (0 == strcmp(int_list_attribute, "wordStarts"))
    return ax::IntListAttribute::kWordStarts;
  if (0 == strcmp(int_list_attribute, "wordEnds"))
    return ax::IntListAttribute::kWordEnds;
  if (0 == strcmp(int_list_attribute, "customActionIds"))
    return ax::IntListAttribute::kCustomActionIds;
  return ax::IntListAttribute::kNone;
}

const char* ToString(ax::StringListAttribute string_list_attribute) {
  switch (string_list_attribute) {
    case ax::StringListAttribute::kNone:
      return "none";
    case ax::StringListAttribute::kCustomActionDescriptions:
      return "customActionDescriptions";
  }

  return "";
}

ax::StringListAttribute ParseStringListAttribute(
    const char* string_list_attribute) {
  if (0 == strcmp(string_list_attribute, "none"))
    return ax::StringListAttribute::kNone;
  if (0 == strcmp(string_list_attribute, "customActionDescriptions"))
    return ax::StringListAttribute::kCustomActionDescriptions;
  return ax::StringListAttribute::kNone;
}

const char* ToString(ax::ListStyle list_style) {
  switch (list_style) {
    case ax::ListStyle::kNone:
      return "none";
    case ax::ListStyle::kCircle:
      return "circle";
    case ax::ListStyle::kDisc:
      return "disc";
    case ax::ListStyle::kImage:
      return "image";
    case ax::ListStyle::kNumeric:
      return "numeric";
    case ax::ListStyle::kOther:
      return "other";
    case ax::ListStyle::kSquare:
      return "square";
  }

  return "";
}

ax::ListStyle ParseListStyle(const char* list_style) {
  if (0 == strcmp(list_style, "none"))
    return ax::ListStyle::kNone;
  if (0 == strcmp(list_style, "circle"))
    return ax::ListStyle::kCircle;
  if (0 == strcmp(list_style, "disc"))
    return ax::ListStyle::kDisc;
  if (0 == strcmp(list_style, "image"))
    return ax::ListStyle::kImage;
  if (0 == strcmp(list_style, "numeric"))
    return ax::ListStyle::kNumeric;
  if (0 == strcmp(list_style, "other"))
    return ax::ListStyle::kOther;
  if (0 == strcmp(list_style, "square"))
    return ax::ListStyle::kSquare;
  return ax::ListStyle::kNone;
}

const char* ToString(ax::MarkerType marker_type) {
  switch (marker_type) {
    case ax::MarkerType::kNone:
      return "none";
    case ax::MarkerType::kSpelling:
      return "spelling";
    case ax::MarkerType::kGrammar:
      return "grammar";
    case ax::MarkerType::kTextMatch:
      return "textMatch";
    case ax::MarkerType::kActiveSuggestion:
      return "activeSuggestion";
    case ax::MarkerType::kSuggestion:
      return "suggestion";
  }

  return "";
}

ax::MarkerType ParseMarkerType(const char* marker_type) {
  if (0 == strcmp(marker_type, "none"))
    return ax::MarkerType::kNone;
  if (0 == strcmp(marker_type, "spelling"))
    return ax::MarkerType::kSpelling;
  if (0 == strcmp(marker_type, "grammar"))
    return ax::MarkerType::kGrammar;
  if (0 == strcmp(marker_type, "textMatch"))
    return ax::MarkerType::kTextMatch;
  if (0 == strcmp(marker_type, "activeSuggestion"))
    return ax::MarkerType::kActiveSuggestion;
  if (0 == strcmp(marker_type, "suggestion"))
    return ax::MarkerType::kSuggestion;
  return ax::MarkerType::kNone;
}

const char* ToString(ax::MoveDirection move_direction) {
  switch (move_direction) {
    case ax::MoveDirection::kForward:
      return "forward";
    case ax::MoveDirection::kBackward:
      return "backward";
  }

  return "";
}

ax::MoveDirection ParseMoveDirection(const char* move_direction) {
  if (0 == strcmp(move_direction, "forward"))
    return ax::MoveDirection::kForward;
  if (0 == strcmp(move_direction, "backward"))
    return ax::MoveDirection::kBackward;
  return ax::MoveDirection::kForward;
}

const char* ToString(ax::Command command) {
  switch (command) {
    case ax::Command::kClearSelection:
      return "clearSelection";
    case ax::Command::kCut:
      return "cut";
    case ax::Command::kDelete:
      return "delete";
    case ax::Command::kDictate:
      return "dictate";
    case ax::Command::kExtendSelection:
      return "extendSelection";
    case ax::Command::kFormat:
      return "format";
    case ax::Command::kInsert:
      return "insert";
    case ax::Command::kMarker:
      return "marker";
    case ax::Command::kMoveSelection:
      return "moveSelection";
    case ax::Command::kPaste:
      return "paste";
    case ax::Command::kReplace:
      return "replace";
    case ax::Command::kSetSelection:
      return "setSelection";
    case ax::Command::kType:
      return "type";
  }

  return "";
}

ax::Command ParseCommand(const char* command) {
  if (0 == strcmp(command, "clearSelection"))
    return ax::Command::kClearSelection;
  if (0 == strcmp(command, "cut"))
    return ax::Command::kCut;
  if (0 == strcmp(command, "delete"))
    return ax::Command::kDelete;
  if (0 == strcmp(command, "dictate"))
    return ax::Command::kDictate;
  if (0 == strcmp(command, "extendSelection"))
    return ax::Command::kExtendSelection;
  if (0 == strcmp(command, "format"))
    return ax::Command::kFormat;
  if (0 == strcmp(command, "insert"))
    return ax::Command::kInsert;
  if (0 == strcmp(command, "marker"))
    return ax::Command::kMarker;
  if (0 == strcmp(command, "moveSelection"))
    return ax::Command::kMoveSelection;
  if (0 == strcmp(command, "paste"))
    return ax::Command::kPaste;
  if (0 == strcmp(command, "replace"))
    return ax::Command::kReplace;
  if (0 == strcmp(command, "setSelection"))
    return ax::Command::kSetSelection;
  if (0 == strcmp(command, "type"))
    return ax::Command::kType;

  // Return the default command.
  return ax::Command::kType;
}

const char* ToString(ax::TextBoundary text_boundary) {
  switch (text_boundary) {
    case ax::TextBoundary::kCharacter:
      return "character";
    case ax::TextBoundary::kFormat:
      return "format";
    case ax::TextBoundary::kLineEnd:
      return "lineEnd";
    case ax::TextBoundary::kLineStart:
      return "lineStart";
    case ax::TextBoundary::kLineStartOrEnd:
      return "lineStartOrEnd";
    case ax::TextBoundary::kObject:
      return "object";
    case ax::TextBoundary::kPageEnd:
      return "pageEnd";
    case ax::TextBoundary::kPageStart:
      return "pageStart";
    case ax::TextBoundary::kPageStartOrEnd:
      return "pageStartOrEnd";
    case ax::TextBoundary::kParagraphEnd:
      return "paragraphEnd";
    case ax::TextBoundary::kParagraphStart:
      return "paragraphStart";
    case ax::TextBoundary::kParagraphStartOrEnd:
      return "paragraphStartOrEnd";
    case ax::TextBoundary::kSentenceEnd:
      return "sentenceEnd";
    case ax::TextBoundary::kSentenceStart:
      return "sentenceStart";
    case ax::TextBoundary::kSentenceStartOrEnd:
      return "sentenceStartOrEnd";
    case ax::TextBoundary::kWebPage:
      return "webPage";
    case ax::TextBoundary::kWordEnd:
      return "wordEnd";
    case ax::TextBoundary::kWordStart:
      return "wordStart";
    case ax::TextBoundary::kWordStartOrEnd:
      return "wordStartOrEnd";
  }

  return "";
}

ax::TextBoundary ParseTextBoundary(const char* text_boundary) {
  if (0 == strcmp(text_boundary, "object"))
    return ax::TextBoundary::kObject;
  if (0 == strcmp(text_boundary, "character"))
    return ax::TextBoundary::kCharacter;
  if (0 == strcmp(text_boundary, "format"))
    return ax::TextBoundary::kFormat;
  if (0 == strcmp(text_boundary, "lineEnd"))
    return ax::TextBoundary::kLineEnd;
  if (0 == strcmp(text_boundary, "lineStart"))
    return ax::TextBoundary::kLineStart;
  if (0 == strcmp(text_boundary, "lineStartOrEnd"))
    return ax::TextBoundary::kLineStartOrEnd;
  if (0 == strcmp(text_boundary, "pageEnd"))
    return ax::TextBoundary::kPageEnd;
  if (0 == strcmp(text_boundary, "pageStart"))
    return ax::TextBoundary::kPageStart;
  if (0 == strcmp(text_boundary, "pageStartOrEnd"))
    return ax::TextBoundary::kPageStartOrEnd;
  if (0 == strcmp(text_boundary, "paragraphEnd"))
    return ax::TextBoundary::kParagraphEnd;
  if (0 == strcmp(text_boundary, "paragraphStart"))
    return ax::TextBoundary::kParagraphStart;
  if (0 == strcmp(text_boundary, "paragraphStartOrEnd"))
    return ax::TextBoundary::kParagraphStartOrEnd;
  if (0 == strcmp(text_boundary, "sentenceEnd"))
    return ax::TextBoundary::kSentenceEnd;
  if (0 == strcmp(text_boundary, "sentenceStart"))
    return ax::TextBoundary::kSentenceStart;
  if (0 == strcmp(text_boundary, "sentenceStartOrEnd"))
    return ax::TextBoundary::kSentenceStartOrEnd;
  if (0 == strcmp(text_boundary, "webPage"))
    return ax::TextBoundary::kWebPage;
  if (0 == strcmp(text_boundary, "wordEnd"))
    return ax::TextBoundary::kWordEnd;
  if (0 == strcmp(text_boundary, "wordStart"))
    return ax::TextBoundary::kWordStart;
  if (0 == strcmp(text_boundary, "wordStartOrEnd"))
    return ax::TextBoundary::kWordStartOrEnd;
  return ax::TextBoundary::kObject;
}

const char* ToString(ax::TextDecorationStyle text_decoration_style) {
  switch (text_decoration_style) {
    case ax::TextDecorationStyle::kNone:
      return "none";
    case ax::TextDecorationStyle::kSolid:
      return "solid";
    case ax::TextDecorationStyle::kDashed:
      return "dashed";
    case ax::TextDecorationStyle::kDotted:
      return "dotted";
    case ax::TextDecorationStyle::kDouble:
      return "double";
    case ax::TextDecorationStyle::kWavy:
      return "wavy";
  }

  return "";
}

ax::TextDecorationStyle ParseTextDecorationStyle(
    const char* text_decoration_style) {
  if (0 == strcmp(text_decoration_style, "none"))
    return ax::TextDecorationStyle::kNone;
  if (0 == strcmp(text_decoration_style, "solid"))
    return ax::TextDecorationStyle::kSolid;
  if (0 == strcmp(text_decoration_style, "dashed"))
    return ax::TextDecorationStyle::kDashed;
  if (0 == strcmp(text_decoration_style, "dotted"))
    return ax::TextDecorationStyle::kDotted;
  if (0 == strcmp(text_decoration_style, "double"))
    return ax::TextDecorationStyle::kDouble;
  if (0 == strcmp(text_decoration_style, "wavy"))
    return ax::TextDecorationStyle::kWavy;
  return ax::TextDecorationStyle::kNone;
}

const char* ToString(ax::TextAlign text_align) {
  switch (text_align) {
    case ax::TextAlign::kNone:
      return "none";
    case ax::TextAlign::kLeft:
      return "left";
    case ax::TextAlign::kRight:
      return "right";
    case ax::TextAlign::kCenter:
      return "center";
    case ax::TextAlign::kJustify:
      return "justify";
  }

  return "";
}

ax::TextAlign ParseTextAlign(const char* text_align) {
  if (0 == strcmp(text_align, "none"))
    return ax::TextAlign::kNone;
  if (0 == strcmp(text_align, "left"))
    return ax::TextAlign::kLeft;
  if (0 == strcmp(text_align, "right"))
    return ax::TextAlign::kRight;
  if (0 == strcmp(text_align, "center"))
    return ax::TextAlign::kCenter;
  if (0 == strcmp(text_align, "justify"))
    return ax::TextAlign::kJustify;
  return ax::TextAlign::kNone;
}

const char* ToString(ax::WritingDirection text_direction) {
  switch (text_direction) {
    case ax::WritingDirection::kNone:
      return "none";
    case ax::WritingDirection::kLtr:
      return "ltr";
    case ax::WritingDirection::kRtl:
      return "rtl";
    case ax::WritingDirection::kTtb:
      return "ttb";
    case ax::WritingDirection::kBtt:
      return "btt";
  }

  return "";
}

ax::WritingDirection ParseTextDirection(const char* text_direction) {
  if (0 == strcmp(text_direction, "none"))
    return ax::WritingDirection::kNone;
  if (0 == strcmp(text_direction, "ltr"))
    return ax::WritingDirection::kLtr;
  if (0 == strcmp(text_direction, "rtl"))
    return ax::WritingDirection::kRtl;
  if (0 == strcmp(text_direction, "ttb"))
    return ax::WritingDirection::kTtb;
  if (0 == strcmp(text_direction, "btt"))
    return ax::WritingDirection::kBtt;
  return ax::WritingDirection::kNone;
}

const char* ToString(ax::TextPosition text_position) {
  switch (text_position) {
    case ax::TextPosition::kNone:
      return "none";
    case ax::TextPosition::kSubscript:
      return "subscript";
    case ax::TextPosition::kSuperscript:
      return "superscript";
  }

  return "";
}

ax::TextPosition ParseTextPosition(const char* text_position) {
  if (0 == strcmp(text_position, "none"))
    return ax::TextPosition::kNone;
  if (0 == strcmp(text_position, "subscript"))
    return ax::TextPosition::kSubscript;
  if (0 == strcmp(text_position, "superscript"))
    return ax::TextPosition::kSuperscript;
  return ax::TextPosition::kNone;
}

const char* ToString(ax::TextStyle text_style) {
  switch (text_style) {
    case ax::TextStyle::kNone:
      return "none";
    case ax::TextStyle::kBold:
      return "bold";
    case ax::TextStyle::kItalic:
      return "italic";
    case ax::TextStyle::kUnderline:
      return "underline";
    case ax::TextStyle::kLineThrough:
      return "lineThrough";
    case ax::TextStyle::kOverline:
      return "overline";
    case ax::TextStyle::kMaxValue:
    case ax::TextStyle::kMinValue:
     break;
  }

  return "";
}

ax::TextStyle ParseTextStyle(const char* text_style) {
  if (0 == strcmp(text_style, "none"))
    return ax::TextStyle::kNone;
  if (0 == strcmp(text_style, "bold"))
    return ax::TextStyle::kBold;
  if (0 == strcmp(text_style, "italic"))
    return ax::TextStyle::kItalic;
  if (0 == strcmp(text_style, "underline"))
    return ax::TextStyle::kUnderline;
  if (0 == strcmp(text_style, "lineThrough"))
    return ax::TextStyle::kLineThrough;
  if (0 == strcmp(text_style, "overline"))
    return ax::TextStyle::kOverline;
  return ax::TextStyle::kNone;
}

const char* ToString(ax::AriaCurrentState aria_current_state) {
  switch (aria_current_state) {
    case ax::AriaCurrentState::kNone:
      return "none";
    case ax::AriaCurrentState::kFalse:
      return "false";
    case ax::AriaCurrentState::kTrue:
      return "true";
    case ax::AriaCurrentState::kPage:
      return "page";
    case ax::AriaCurrentState::kStep:
      return "step";
    case ax::AriaCurrentState::kLocation:
      return "location";
    case ax::AriaCurrentState::kUnclippedLocation:
      return "unclippedLocation";
    case ax::AriaCurrentState::kDate:
      return "date";
    case ax::AriaCurrentState::kTime:
      return "time";
  }

  return "";
}

ax::AriaCurrentState ParseAriaCurrentState(
    const char* aria_current_state) {
  if (0 == strcmp(aria_current_state, "none"))
    return ax::AriaCurrentState::kNone;
  if (0 == strcmp(aria_current_state, "false"))
    return ax::AriaCurrentState::kFalse;
  if (0 == strcmp(aria_current_state, "true"))
    return ax::AriaCurrentState::kTrue;
  if (0 == strcmp(aria_current_state, "page"))
    return ax::AriaCurrentState::kPage;
  if (0 == strcmp(aria_current_state, "step"))
    return ax::AriaCurrentState::kStep;
  if (0 == strcmp(aria_current_state, "location"))
    return ax::AriaCurrentState::kLocation;
  if (0 == strcmp(aria_current_state, "unclippedLocation"))
    return ax::AriaCurrentState::kUnclippedLocation;
  if (0 == strcmp(aria_current_state, "date"))
    return ax::AriaCurrentState::kDate;
  if (0 == strcmp(aria_current_state, "time"))
    return ax::AriaCurrentState::kTime;
  return ax::AriaCurrentState::kNone;
}

const char* ToString(ax::HasPopup has_popup) {
  switch (has_popup) {
    case ax::HasPopup::kFalse:
      return "";
    case ax::HasPopup::kTrue:
      return "true";
    case ax::HasPopup::kMenu:
      return "menu";
    case ax::HasPopup::kListbox:
      return "listbox";
    case ax::HasPopup::kTree:
      return "tree";
    case ax::HasPopup::kGrid:
      return "grid";
    case ax::HasPopup::kDialog:
      return "dialog";
  }

  return "";
}

ax::HasPopup ParseHasPopup(const char* has_popup) {
  if (0 == strcmp(has_popup, "true"))
    return ax::HasPopup::kTrue;
  if (0 == strcmp(has_popup, "menu"))
    return ax::HasPopup::kMenu;
  if (0 == strcmp(has_popup, "listbox"))
    return ax::HasPopup::kListbox;
  if (0 == strcmp(has_popup, "tree"))
    return ax::HasPopup::kTree;
  if (0 == strcmp(has_popup, "grid"))
    return ax::HasPopup::kGrid;
  if (0 == strcmp(has_popup, "dialog"))
    return ax::HasPopup::kDialog;

  return ax::HasPopup::kFalse;
}

const char* ToString(ax::InvalidState invalid_state) {
  switch (invalid_state) {
    case ax::InvalidState::kNone:
      return "none";
    case ax::InvalidState::kFalse:
      return "false";
    case ax::InvalidState::kTrue:
      return "true";
    case ax::InvalidState::kOther:
      return "other";
  }

  return "";
}

ax::InvalidState ParseInvalidState(const char* invalid_state) {
  if (0 == strcmp(invalid_state, "none"))
    return ax::InvalidState::kNone;
  if (0 == strcmp(invalid_state, "false"))
    return ax::InvalidState::kFalse;
  if (0 == strcmp(invalid_state, "true"))
    return ax::InvalidState::kTrue;
  if (0 == strcmp(invalid_state, "other"))
    return ax::InvalidState::kOther;
  return ax::InvalidState::kNone;
}

const char* ToString(ax::Restriction restriction) {
  switch (restriction) {
    case ax::Restriction::kNone:
      return "none";
    case ax::Restriction::kReadOnly:
      return "readOnly";
    case ax::Restriction::kDisabled:
      return "disabled";
  }

  return "";
}

ax::Restriction ParseRestriction(const char* restriction) {
  if (0 == strcmp(restriction, "none"))
    return ax::Restriction::kNone;
  if (0 == strcmp(restriction, "readOnly"))
    return ax::Restriction::kReadOnly;
  if (0 == strcmp(restriction, "disabled"))
    return ax::Restriction::kDisabled;
  return ax::Restriction::kNone;
}

const char* ToString(ax::CheckedState checked_state) {
  switch (checked_state) {
    case ax::CheckedState::kNone:
      return "none";
    case ax::CheckedState::kFalse:
      return "false";
    case ax::CheckedState::kTrue:
      return "true";
    case ax::CheckedState::kMixed:
      return "mixed";
  }

  return "";
}

ax::CheckedState ParseCheckedState(const char* checked_state) {
  if (0 == strcmp(checked_state, "none"))
    return ax::CheckedState::kNone;
  if (0 == strcmp(checked_state, "false"))
    return ax::CheckedState::kFalse;
  if (0 == strcmp(checked_state, "true"))
    return ax::CheckedState::kTrue;
  if (0 == strcmp(checked_state, "mixed"))
    return ax::CheckedState::kMixed;
  return ax::CheckedState::kNone;
}

const char* ToString(ax::SortDirection sort_direction) {
  switch (sort_direction) {
    case ax::SortDirection::kNone:
      return "none";
    case ax::SortDirection::kUnsorted:
      return "unsorted";
    case ax::SortDirection::kAscending:
      return "ascending";
    case ax::SortDirection::kDescending:
      return "descending";
    case ax::SortDirection::kOther:
      return "other";
  }

  return "";
}

ax::SortDirection ParseSortDirection(const char* sort_direction) {
  if (0 == strcmp(sort_direction, "none"))
    return ax::SortDirection::kNone;
  if (0 == strcmp(sort_direction, "unsorted"))
    return ax::SortDirection::kUnsorted;
  if (0 == strcmp(sort_direction, "ascending"))
    return ax::SortDirection::kAscending;
  if (0 == strcmp(sort_direction, "descending"))
    return ax::SortDirection::kDescending;
  if (0 == strcmp(sort_direction, "other"))
    return ax::SortDirection::kOther;
  return ax::SortDirection::kNone;
}

const char* ToString(ax::NameFrom name_from) {
  switch (name_from) {
    case ax::NameFrom::kNone:
      return "none";
    case ax::NameFrom::kUninitialized:
      return "uninitialized";
    case ax::NameFrom::kAttribute:
      return "attribute";
    case ax::NameFrom::kAttributeExplicitlyEmpty:
      return "attributeExplicitlyEmpty";
    case ax::NameFrom::kCaption:
      return "caption";
    case ax::NameFrom::kContents:
      return "contents";
    case ax::NameFrom::kPlaceholder:
      return "placeholder";
    case ax::NameFrom::kRelatedElement:
      return "relatedElement";
    case ax::NameFrom::kTitle:
      return "title";
    case ax::NameFrom::kValue:
      return "value";
  }

  return "";
}

ax::NameFrom ParseNameFrom(const char* name_from) {
  if (0 == strcmp(name_from, "none"))
    return ax::NameFrom::kNone;
  if (0 == strcmp(name_from, "uninitialized"))
    return ax::NameFrom::kUninitialized;
  if (0 == strcmp(name_from, "attribute"))
    return ax::NameFrom::kAttribute;
  if (0 == strcmp(name_from, "attributeExplicitlyEmpty"))
    return ax::NameFrom::kAttributeExplicitlyEmpty;
  if (0 == strcmp(name_from, "caption"))
    return ax::NameFrom::kCaption;
  if (0 == strcmp(name_from, "contents"))
    return ax::NameFrom::kContents;
  if (0 == strcmp(name_from, "placeholder"))
    return ax::NameFrom::kPlaceholder;
  if (0 == strcmp(name_from, "relatedElement"))
    return ax::NameFrom::kRelatedElement;
  if (0 == strcmp(name_from, "title"))
    return ax::NameFrom::kTitle;
  if (0 == strcmp(name_from, "value"))
    return ax::NameFrom::kValue;
  return ax::NameFrom::kNone;
}

const char* ToString(ax::DescriptionFrom description_from) {
  switch (description_from) {
    case ax::DescriptionFrom::kNone:
      return "none";
    case ax::DescriptionFrom::kUninitialized:
      return "uninitialized";
    case ax::DescriptionFrom::kAttribute:
      return "attribute";
    case ax::DescriptionFrom::kContents:
      return "contents";
    case ax::DescriptionFrom::kRelatedElement:
      return "relatedElement";
    case ax::DescriptionFrom::kTitle:
      return "title";
  }

  return "";
}

ax::DescriptionFrom ParseDescriptionFrom(const char* description_from) {
  if (0 == strcmp(description_from, "none"))
    return ax::DescriptionFrom::kNone;
  if (0 == strcmp(description_from, "uninitialized"))
    return ax::DescriptionFrom::kUninitialized;
  if (0 == strcmp(description_from, "attribute"))
    return ax::DescriptionFrom::kAttribute;
  if (0 == strcmp(description_from, "contents"))
    return ax::DescriptionFrom::kContents;
  if (0 == strcmp(description_from, "relatedElement"))
    return ax::DescriptionFrom::kRelatedElement;
  if (0 == strcmp(description_from, "title"))
    return ax::DescriptionFrom::kTitle;
  return ax::DescriptionFrom::kNone;
}

const char* ToString(ax::EventFrom event_from) {
  switch (event_from) {
    case ax::EventFrom::kNone:
      return "none";
    case ax::EventFrom::kUser:
      return "user";
    case ax::EventFrom::kPage:
      return "page";
    case ax::EventFrom::kAction:
      return "action";
  }

  return "";
}

ax::EventFrom ParseEventFrom(const char* event_from) {
  if (0 == strcmp(event_from, "none"))
    return ax::EventFrom::kNone;
  if (0 == strcmp(event_from, "user"))
    return ax::EventFrom::kUser;
  if (0 == strcmp(event_from, "page"))
    return ax::EventFrom::kPage;
  if (0 == strcmp(event_from, "action"))
    return ax::EventFrom::kAction;
  return ax::EventFrom::kNone;
}

const char* ToString(ax::Gesture gesture) {
  switch (gesture) {
    case ax::Gesture::kNone:
      return "none";
    case ax::Gesture::kClick:
      return "click";
    case ax::Gesture::kSwipeLeft1:
      return "swipeLeft1";
    case ax::Gesture::kSwipeUp1:
      return "swipeUp1";
    case ax::Gesture::kSwipeRight1:
      return "swipeRight1";
    case ax::Gesture::kSwipeDown1:
      return "swipeDown1";
    case ax::Gesture::kSwipeLeft2:
      return "swipeLeft2";
    case ax::Gesture::kSwipeUp2:
      return "swipeUp2";
    case ax::Gesture::kSwipeRight2:
      return "swipeRight2";
    case ax::Gesture::kSwipeDown2:
      return "swipeDown2";
    case ax::Gesture::kSwipeLeft3:
      return "swipeLeft3";
    case ax::Gesture::kSwipeUp3:
      return "swipeUp3";
    case ax::Gesture::kSwipeRight3:
      return "swipeRight3";
    case ax::Gesture::kSwipeDown3:
      return "swipeDown3";
    case ax::Gesture::kSwipeLeft4:
      return "swipeLeft4";
    case ax::Gesture::kSwipeUp4:
      return "swipeUp4";
    case ax::Gesture::kSwipeRight4:
      return "swipeRight4";
    case ax::Gesture::kSwipeDown4:
      return "swipeDown4";
    case ax::Gesture::kTap2:
      return "tap2";
    case ax::Gesture::kTap3:
      return "tap3";
    case ax::Gesture::kTap4:
      return "tap4";
    case ax::Gesture::kTouchExplore:
      return "touchExplore";
  }

  return "";
}

ax::Gesture ParseGesture(const char* gesture) {
  if (0 == strcmp(gesture, "none"))
    return ax::Gesture::kNone;
  if (0 == strcmp(gesture, "click"))
    return ax::Gesture::kClick;
  if (0 == strcmp(gesture, "swipeLeft1"))
    return ax::Gesture::kSwipeLeft1;
  if (0 == strcmp(gesture, "swipeUp1"))
    return ax::Gesture::kSwipeUp1;
  if (0 == strcmp(gesture, "swipeRight1"))
    return ax::Gesture::kSwipeRight1;
  if (0 == strcmp(gesture, "swipeDown1"))
    return ax::Gesture::kSwipeDown1;
  if (0 == strcmp(gesture, "swipeLeft2"))
    return ax::Gesture::kSwipeLeft2;
  if (0 == strcmp(gesture, "swipeUp2"))
    return ax::Gesture::kSwipeUp2;
  if (0 == strcmp(gesture, "swipeRight2"))
    return ax::Gesture::kSwipeRight2;
  if (0 == strcmp(gesture, "swipeDown2"))
    return ax::Gesture::kSwipeDown2;
  if (0 == strcmp(gesture, "swipeLeft3"))
    return ax::Gesture::kSwipeLeft3;
  if (0 == strcmp(gesture, "swipeUp3"))
    return ax::Gesture::kSwipeUp3;
  if (0 == strcmp(gesture, "swipeRight3"))
    return ax::Gesture::kSwipeRight3;
  if (0 == strcmp(gesture, "swipeDown3"))
    return ax::Gesture::kSwipeDown3;
  if (0 == strcmp(gesture, "swipeLeft4"))
    return ax::Gesture::kSwipeLeft4;
  if (0 == strcmp(gesture, "swipeUp4"))
    return ax::Gesture::kSwipeUp4;
  if (0 == strcmp(gesture, "swipeRight4"))
    return ax::Gesture::kSwipeRight4;
  if (0 == strcmp(gesture, "swipeDown4"))
    return ax::Gesture::kSwipeDown4;
  if (0 == strcmp(gesture, "tap2"))
    return ax::Gesture::kTap2;
  if (0 == strcmp(gesture, "tap3"))
    return ax::Gesture::kTap3;
  if (0 == strcmp(gesture, "tap4"))
    return ax::Gesture::kTap4;
  if (0 == strcmp(gesture, "touchExplore"))
    return ax::Gesture::kTouchExplore;
  return ax::Gesture::kNone;
}

const char* ToString(ax::TextAffinity text_affinity) {
  switch (text_affinity) {
    case ax::TextAffinity::kNone:
      return "none";
    case ax::TextAffinity::kDownstream:
      return "downstream";
    case ax::TextAffinity::kUpstream:
      return "upstream";
  }

  return "";
}

ax::TextAffinity ParseTextAffinity(const char* text_affinity) {
  if (0 == strcmp(text_affinity, "none"))
    return ax::TextAffinity::kNone;
  if (0 == strcmp(text_affinity, "downstream"))
    return ax::TextAffinity::kDownstream;
  if (0 == strcmp(text_affinity, "upstream"))
    return ax::TextAffinity::kUpstream;
  return ax::TextAffinity::kNone;
}

const char* ToString(ax::TreeOrder tree_order) {
  switch (tree_order) {
    case ax::TreeOrder::kNone:
      return "none";
    case ax::TreeOrder::kUndefined:
      return "undefined";
    case ax::TreeOrder::kBefore:
      return "before";
    case ax::TreeOrder::kEqual:
      return "equal";
    case ax::TreeOrder::kAfter:
      return "after";
  }

  return "";
}

ax::TreeOrder ParseTreeOrder(const char* tree_order) {
  if (0 == strcmp(tree_order, "none"))
    return ax::TreeOrder::kNone;
  if (0 == strcmp(tree_order, "undefined"))
    return ax::TreeOrder::kUndefined;
  if (0 == strcmp(tree_order, "before"))
    return ax::TreeOrder::kBefore;
  if (0 == strcmp(tree_order, "equal"))
    return ax::TreeOrder::kEqual;
  if (0 == strcmp(tree_order, "after"))
    return ax::TreeOrder::kAfter;
  return ax::TreeOrder::kNone;
}

const char* ToString(ax::ImageAnnotationStatus status) {
  switch (status) {
    case ax::ImageAnnotationStatus::kNone:
      return "none";
    case ax::ImageAnnotationStatus::kWillNotAnnotateDueToScheme:
      return "kWillNotAnnotateDueToScheme";
    case ax::ImageAnnotationStatus::kIneligibleForAnnotation:
      return "ineligibleForAnnotation";
    case ax::ImageAnnotationStatus::kEligibleForAnnotation:
      return "eligibleForAnnotation";
    case ax::ImageAnnotationStatus::kSilentlyEligibleForAnnotation:
      return "silentlyEligibleForAnnotation";
    case ax::ImageAnnotationStatus::kAnnotationPending:
      return "annotationPending";
    case ax::ImageAnnotationStatus::kAnnotationSucceeded:
      return "annotationSucceeded";
    case ax::ImageAnnotationStatus::kAnnotationEmpty:
      return "annotationEmpty";
    case ax::ImageAnnotationStatus::kAnnotationAdult:
      return "annotationAdult";
    case ax::ImageAnnotationStatus::kAnnotationProcessFailed:
      return "annotationProcessFailed";
  }

  return "";
}

ax::ImageAnnotationStatus ParseImageAnnotationStatus(
    const char* status) {
  if (0 == strcmp(status, "none"))
    return ax::ImageAnnotationStatus::kNone;
  if (0 == strcmp(status, "kWillNotAnnotateDueToScheme"))
    return ax::ImageAnnotationStatus::kWillNotAnnotateDueToScheme;
  if (0 == strcmp(status, "ineligibleForAnnotation"))
    return ax::ImageAnnotationStatus::kIneligibleForAnnotation;
  if (0 == strcmp(status, "eligibleForAnnotation"))
    return ax::ImageAnnotationStatus::kEligibleForAnnotation;
  if (0 == strcmp(status, "silentlyEligibleForAnnotation"))
    return ax::ImageAnnotationStatus::kSilentlyEligibleForAnnotation;
  if (0 == strcmp(status, "annotationPending"))
    return ax::ImageAnnotationStatus::kAnnotationPending;
  if (0 == strcmp(status, "annotationSucceeded"))
    return ax::ImageAnnotationStatus::kAnnotationSucceeded;
  if (0 == strcmp(status, "annotationEmpty"))
    return ax::ImageAnnotationStatus::kAnnotationEmpty;
  if (0 == strcmp(status, "annotationAdult"))
    return ax::ImageAnnotationStatus::kAnnotationAdult;
  if (0 == strcmp(status, "annotationProcessFailed"))
    return ax::ImageAnnotationStatus::kAnnotationProcessFailed;

  return ax::ImageAnnotationStatus::kNone;
}

const char* ToString(ax::Dropeffect dropeffect) {
  switch (dropeffect) {
    case ax::Dropeffect::kCopy:
      return "copy";
    case ax::Dropeffect::kExecute:
      return "execute";
    case ax::Dropeffect::kLink:
      return "link";
    case ax::Dropeffect::kMove:
      return "move";
    case ax::Dropeffect::kPopup:
      return "popup";
    case ax::Dropeffect::kNone:
      return "none";
    case ax::Dropeffect::kMaxValue:
    case ax::Dropeffect::kMinValue:
     break;
  }

  return "";
}

ax::Dropeffect ParseDropeffect(const char* dropeffect) {
  if (0 == strcmp(dropeffect, "copy"))
    return ax::Dropeffect::kCopy;
  if (0 == strcmp(dropeffect, "execute"))
    return ax::Dropeffect::kExecute;
  if (0 == strcmp(dropeffect, "link"))
    return ax::Dropeffect::kLink;
  if (0 == strcmp(dropeffect, "move"))
    return ax::Dropeffect::kMove;
  if (0 == strcmp(dropeffect, "popup"))
    return ax::Dropeffect::kPopup;
  return ax::Dropeffect::kNone;
}

}  // namespace ax
