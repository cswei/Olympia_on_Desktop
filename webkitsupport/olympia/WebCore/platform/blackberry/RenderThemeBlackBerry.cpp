/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#include "config.h"
#include "RenderThemeBlackBerry.h"

#include "Frame.h"
#include "Gradient.h"
#include "GraphicsContext.h"
#include "Path.h"
#include "RenderView.h"
#include "UserAgentStyleSheets.h"

namespace WebCore {

// Sizes (px)
const unsigned smallRadius = 1;
const unsigned largeRadius = 3;
const unsigned lineWidth = 1;
const int marginSize = 4;
const int sliderThumbWidth = 15;
const int sliderThumbHeight = 25;

// Checkbox check scalers
const float checkboxLeftX = 7 / 40.0;
const float checkboxLeftY = 1 / 2.0;
const float checkboxMiddleX = 19 / 50.0;
const float checkboxMiddleY = 7 / 25.0;
const float checkboxRightX = 33 / 40.0;
const float checkboxRightY = 1 / 5.0;
const float checkboxStrokeThickness = 6.5;

// Radio button scaler
const float radioButtonCheckStateScaler = 7 / 30.0;

// Multipliers
const unsigned paddingDivisor = 5;

// Colors
const RGBA32 caretBottom = 0xff2163bf;
const RGBA32 caretTop = 0xff69a5fa;

const RGBA32 regularBottom = 0xffdcdee4;
const RGBA32 regularTop = 0xfff7f2ee;
const RGBA32 hoverBottom = 0xffb5d3fc;
const RGBA32 hoverTop = 0xffcceaff;
const RGBA32 depressedBottom = 0xff3388ff;
const RGBA32 depressedTop = 0xff66a0f2;
const RGBA32 disabledBottom = 0xffe7e7e7;
const RGBA32 disabledTop = 0xffefefef;

const RGBA32 regularBottomOutline = 0xff6e7073;
const RGBA32 regularTopOutline = 0xffb9b8b8;
const RGBA32 hoverBottomOutline = 0xff2163bf;
const RGBA32 hoverTopOutline = 0xff69befa;
const RGBA32 depressedBottomOutline = 0xff0c3d81;
const RGBA32 depressedTopOutline = 0xff1d4d70;
const RGBA32 disabledOutline = 0xffd5d9de;

const RGBA32 rangeSliderRegularBottom = 0xfff6f2ee;
const RGBA32 rangeSliderRegularTop = 0xffdee0e5;
const RGBA32 rangeSliderRollBottom = 0xffc9e8fe;
const RGBA32 rangeSliderRollTop = 0xffb5d3fc;

const RGBA32 rangeSliderRegularBottomOutline = 0xffb9babd;
const RGBA32 rangeSliderRegularTopOutline = 0xffb7b7b7;
const RGBA32 rangeSliderRollBottomOutline = 0xff67abe0;
const RGBA32 rangeSliderRollTopOutline = 0xff69adf9;

const RGBA32 dragRegularLight = 0xfffdfdfd;
const RGBA32 dragRegularDark = 0xffbababa;
const RGBA32 dragRollLight = 0xfff2f2f2;
const RGBA32 dragRollDark = 0xff69a8ff;

const RGBA32 textLink = 0xff006aff;
const RGBA32 selection = 0xff2b8fff;

const RGBA32 blackPen = Color::black;
const RGBA32 outlinePen = 0x80ff7b00;

static PassRefPtr<Gradient> createLinearGradient(RGBA32 top, RGBA32 bottom, const IntPoint& a, const IntPoint& b)
{
    RefPtr<Gradient> gradient = Gradient::create(a, b);
    gradient->addColorStop(0.0, Color(top));
    gradient->addColorStop(1.0, Color(bottom));
    return gradient.release();
}

PassRefPtr<RenderTheme> RenderTheme::themeForPage(Page* page)
{
    static RenderThemeBlackBerry theme;
    return &theme;
}

RenderThemeBlackBerry::RenderThemeBlackBerry()
    : RenderTheme()
    , m_shouldRepaintVerticalCaret(false)
    , m_caretOutlineAppearanceEnabled(false)
{
}

RenderThemeBlackBerry::~RenderThemeBlackBerry()
{
}

String RenderThemeBlackBerry::extraDefaultStyleSheet()
{
    return String(themeBlackBerryUserAgentStyleSheet, sizeof(themeBlackBerryUserAgentStyleSheet));
}

double RenderThemeBlackBerry::caretBlinkInterval() const
{
    return 0.0; // Turn off caret blinking.
}

void RenderThemeBlackBerry::adjustTextColorForCaretMarker(Color& color) const
{
    if (!m_caretOutlineAppearanceEnabled)
        color = Color::white;
}

static PassRefPtr<Range> computeCaretCharacterRange(Frame* frame)
{
    // We want to select the next character with respect to the start of the current selection.
    VisibleSelection startOfCurrentSelection(frame->selection()->start(), frame->selection()->affinity());

    SelectionController trialSelection;
    trialSelection.setSelection(startOfCurrentSelection);
    trialSelection.modify(SelectionController::EXTEND, SelectionController::FORWARD, CharacterGranularity);
    return trialSelection.toNormalizedRange();
}

static inline float computeVerticalCaretWidth(const Font& font)
{
    const TextRun spaceRun(" ");
    return font.floatWidth(spaceRun);
}

static void addCaretMarkerAndRepaint(const Frame* frame, Range* caretTextRange)
{
    ASSERT(caretTextRange->endOffset() - caretTextRange->startOffset() <= 1); // The caret marker highlights at most one character.

    ExceptionCode ec = 0;
    caretTextRange->startContainer(ec)->document()->addMarker(caretTextRange, DocumentMarker::Caret);
    ASSERT(!ec);
    if (frame->selection()->caretRenderer())
        frame->selection()->caretRenderer()->repaint();
}

static void paintSolidCaret(GraphicsContext* context, const FloatRect& caretRect)
{
    IntRect caretIntRect(caretRect); // We do not need to be precise when creating the gradient.
    context->save();
    context->setFillGradient(createLinearGradient(caretTop, caretBottom, caretIntRect.topRight(), caretIntRect.bottomRight()));
    context->fillRect(caretRect);
    context->restore();
}

static bool shouldRepaintVerticalCaret(Range* caretTextRange)
{
    // We repaint the vertical caret for line breaks so that we match the BlackBerry OS.
    ExceptionCode ec = 0;
    String nextCharacter = caretTextRange->toString(ec); // Will be empty for a <br/> in a content-editable HTML element.
    ASSERT(!ec);
    return caretTextRange->startOffset() == caretTextRange->endOffset() || nextCharacter.isEmpty() || nextCharacter[0] == '\n';
}

void RenderThemeBlackBerry::paintCaret(GraphicsContext* context, const IntRect& caretRect, const Element* rootEditableElement)
{
    Frame* frame = rootEditableElement && rootEditableElement->document() ? rootEditableElement->document()->frame() : 0;
    if (!frame)
        return;

    SelectionController* selection = frame->selection();
    if (!selection || !selection->start().node())
        return;

    RefPtr<Range> caretTextRange = computeCaretCharacterRange(frame);
    if (!caretTextRange)
        return;

    // Because RenderThemeBlackBerry::repaintCaret may cause a repaint of a node (such as when we
    // add the caret marker) we may get called even though the caret marker was already added and
    // painted (i.e. caretTextRange == m_oldCaretTextRange). So, we do not need to add a caret
    // marker/paint it again.
    bool canAddCaretMarker = !areRangesEqual(caretTextRange.get(), m_oldCaretTextRange.get());

    ExceptionCode ec = 0;
    m_oldCaretTextRange = caretTextRange->cloneRange(ec);
    ASSERT(!ec);

    if (shouldRepaintVerticalCaret(caretTextRange.get())) {
        // Paint the vertical caret.
        if (rootEditableElement->renderer()) {
            Font caretFont = rootEditableElement->renderer()->style()->font();
            FloatRect verticalCaretRect(caretRect);
            verticalCaretRect.setWidth(computeVerticalCaretWidth(caretFont));
            paintSolidCaret(context, verticalCaretRect);
            m_shouldRepaintVerticalCaret = true;
        }
    } else if (canAddCaretMarker)
        addCaretMarkerAndRepaint(frame, caretTextRange.get());
}

void RenderThemeBlackBerry::repaintCaret(RenderView* view, const IntRect& caretRect, CaretVisibility caretVisibility)
{
    // Notice, this method is always called twice by SelectionController on a repaint. The first time
    // is to clear the old caret rectangle. The second time is to paint the new caret rectangle.
    Frame* frame = view && view->frameView() ? view->frameView()->frame() : 0;
    if (!frame)
        return;

    // FIXME: We only need to clear the caret marker when either we switch to the vertical caret or
    // caretVisibility == RenderTheme::CaretHidden. For the case where the caret marker is moved we
    // can compute the move and update the caret marker's position, instead of removing it then
    // adding a new marker.
    if (frame->document())
        frame->document()->removeMarkers(DocumentMarker::Caret);

    SelectionController* selection = frame->selection();
    if (!selection || !selection->start().node())
        return;

    RefPtr<Range> caretTextRange = computeCaretCharacterRange(frame);
    if (!caretTextRange)
        return;

    ExceptionCode ec = 0;
    m_oldCaretTextRange = caretTextRange->cloneRange(ec);
    ASSERT(!ec);

    // The order of the following if-statements have been explicitly chosen so that we 1) either repaint
    // the vertical caret or the caret marker (but not both) and 2) always repaint the vertical caret
    // rectangle so as to clear it should we switch to the caret marker (indicated by m_shouldRepaintVerticalCaret).
    bool willRepaintVerticalCaret = shouldRepaintVerticalCaret(caretTextRange.get());
    // FIXME: We should rename m_shouldRepaintVerticalCaret to m_lastCaretWasVerticalCaret.
    if (!selection->isCaret() || m_shouldRepaintVerticalCaret || willRepaintVerticalCaret) {
        // Repaint the vertical caret. When !selection->isCaret() then the vertical caret must be cleared
        // because we may have started a selection from the end of the text field.
        Element* rootEditableElement = selection->rootEditableElement();
        if (rootEditableElement && rootEditableElement->renderer()) {
            int verticalCaretWidth = lroundf(computeVerticalCaretWidth(rootEditableElement->renderer()->style()->font()));
            // We need to re-compute the repaint rect for the caret since the width of our custom caret is larger
            // than the standard WebKit caret.
            IntRect verticalCaretRepaintRect = caretRect;
            verticalCaretRepaintRect.setWidth(verticalCaretWidth);
            view->repaintViewRectangle(SelectionController::repaintRectForCaret(verticalCaretRepaintRect), false);
        }
        m_shouldRepaintVerticalCaret = false;
    }

    if (!selection->isCaret())
        return;

    if (m_shouldRepaintVerticalCaret || willRepaintVerticalCaret)
        return;

    if (caretVisibility == RenderTheme::CaretVisible)
        addCaretMarkerAndRepaint(frame, caretTextRange.get());
}

// Called by InlineTextBox::paintCaretMarker.
void RenderThemeBlackBerry::paintCaretMarker(GraphicsContext* context, const FloatRect& caretRect, const Font&)
{
    if (!m_caretOutlineAppearanceEnabled) {
        paintSolidCaret(context, caretRect);
        return;
    }

    // Paint outline caret.
    context->save();
    context->setStrokeStyle(SolidStroke);

    // A stroke thickness of 0.25 seems to give a reasonable outline while not occluding narrow characters.
    context->setStrokeThickness(0.25);

    context->setStrokeColor(Color::black, DeviceColorSpace);
    context->strokeRect(caretRect);
    context->restore();
}

void RenderThemeBlackBerry::systemFont(int, FontDescription&) const
{
    // no-op
}

void RenderThemeBlackBerry::setButtonStyle(RenderStyle* style) const
{
    Length vertPadding(int(style->fontSize() / paddingDivisor), Fixed);
    style->setPaddingTop(vertPadding);
    style->setPaddingBottom(vertPadding);

    Length margin(marginSize, Fixed);
    style->setMarginBottom(margin);
    style->setMarginRight(margin);
}

void RenderThemeBlackBerry::adjustButtonStyle(CSSStyleSelector*, RenderStyle* style, Element*) const
{
    setButtonStyle(style);
    style->setCursor(CURSOR_WEBKIT_GRAB);
}

void RenderThemeBlackBerry::adjustTextAreaStyle(CSSStyleSelector*, RenderStyle* style, Element*) const
{
    setButtonStyle(style);
}

bool RenderThemeBlackBerry::paintTextArea(RenderObject* object, const RenderObject::PaintInfo& info, const IntRect& rect)
{
    return paintTextField(object, info, rect);
}

void RenderThemeBlackBerry::adjustTextFieldStyle(CSSStyleSelector* css, RenderStyle* style, Element* e) const
{
    setButtonStyle(style);
}

bool RenderThemeBlackBerry::paintTextField(RenderObject* object, const RenderObject::PaintInfo& info, const IntRect& rect)
{
    // FIXME: We should tint on a pressed state to be consistent with the behavior of other controls.
    // However, we don't seem to receive a state change on mouse up and only re-render on a hovered state
    // change or unfocus. See RIM Bug 664.
    ASSERT(info.context);
    GraphicsContext* context = info.context;

    context->save();
    context->setStrokeStyle(SolidStroke);
    context->setStrokeThickness(lineWidth);
    if (!isEnabled(object))
        context->setStrokeColor(disabledOutline, DeviceColorSpace);
    else if (isHovered(object) || isFocused(object))
        context->setStrokeGradient(createLinearGradient(hoverTopOutline, hoverBottomOutline, rect.topRight(), rect.bottomRight()));
    else
        context->setStrokeGradient(createLinearGradient(regularTopOutline, regularBottomOutline, rect.topRight(), rect.bottomRight()));

    context->beginPath();
    Path textFieldRoundedRectangle = Path::createRoundedRectangle(rect, FloatSize(largeRadius, largeRadius));
    context->addPath(textFieldRoundedRectangle);
    context->strokePath();
    context->restore();
    return false;
}

void RenderThemeBlackBerry::adjustSearchFieldStyle(CSSStyleSelector* css, RenderStyle* style, Element* e) const
{
    setButtonStyle(style);
}

bool RenderThemeBlackBerry::paintSearchField(RenderObject* object, const RenderObject::PaintInfo& info, const IntRect& rect)
{
    return paintTextField(object, info, rect);
}

void RenderThemeBlackBerry::adjustMenuListButtonStyle(CSSStyleSelector*, RenderStyle* style, Element*) const
{
    const int paddingLeft = 8; // From observation, seems to produce a reasonable result
    const int paddingRight = 4; // From observation, seems to produce a reasonable result
    int height = 0;
    const int minHeight = style->fontSize() * 2;

    style->resetPadding();
    style->setMinHeight(Length(minHeight, Fixed));

    if (style->height().isFixed() && (height = style->height().value()) > minHeight)
        style->setPaddingRight(Length(height + paddingRight, Fixed)); 
    else {
        style->setMaxHeight(Length(minHeight, Fixed));
        style->setPaddingRight(Length(minHeight + paddingRight, Fixed));
    }

    style->setPaddingLeft(Length(paddingLeft, Fixed));
}

void RenderThemeBlackBerry::calculateButtonSize(RenderStyle* style) const
{
    int size = style->fontSize();
    if (style->appearance() == CheckboxPart || style->appearance() == RadioPart) {
        style->setWidth(Length(size, Fixed));
        style->setHeight(Length(size, Fixed));
        return;
    }

    // If the width and height are both specified, then we have nothing to do.
    if (!style->width().isIntrinsicOrAuto() && !style->height().isAuto())
        return;

    if (style->width().isIntrinsicOrAuto())
        style->setWidth(Length(size, Fixed));

    if (style->height().isAuto())
        style->setHeight(Length(size, Fixed));
}

bool RenderThemeBlackBerry::paintCheckbox(RenderObject* object, const RenderObject::PaintInfo& info, const IntRect& rect)
{
    return paintButton(object, info, rect);
}

void RenderThemeBlackBerry::setCheckboxSize(RenderStyle* style) const
{
    calculateButtonSize(style);
}

bool RenderThemeBlackBerry::paintRadio(RenderObject* object, const RenderObject::PaintInfo& info, const IntRect& rect)
{
    return paintButton(object, info, rect);
}

void RenderThemeBlackBerry::setRadioSize(RenderStyle* style) const
{
    calculateButtonSize(style);
}

// If this function returns false, WebCore assumes the button is fully decorated
bool RenderThemeBlackBerry::paintButton(RenderObject* object, const RenderObject::PaintInfo& info, const IntRect& rect)
{
    Path path;
    Color check(blackPen);

    ASSERT(info.context);
    info.context->save();

    info.context->setStrokeStyle(SolidStroke);
    info.context->setStrokeThickness(lineWidth);

    if (!isEnabled(object)) {
        info.context->setFillGradient(createLinearGradient(disabledTop, disabledBottom, rect.topRight(), rect.bottomRight()));
        info.context->setStrokeColor(disabledOutline, DeviceColorSpace);
        check = Color(blackPen);
    } else if (isPressed(object)) {
        info.context->setFillGradient(createLinearGradient(depressedTop, depressedBottom, rect.topRight(), rect.bottomRight()));
        info.context->setStrokeGradient(createLinearGradient(depressedTopOutline, depressedBottomOutline, rect.topRight(), rect.bottomRight()));
    } else if (isHovered(object)) {
        info.context->setFillGradient(createLinearGradient(hoverTop, hoverBottom, rect.topRight(), rect.bottomRight()));
        info.context->setStrokeGradient(createLinearGradient(hoverTopOutline, hoverBottomOutline, rect.topRight(), rect.bottomRight()));
    } else {
        info.context->setFillGradient(createLinearGradient(regularTop, regularBottom, rect.topRight(), rect.bottomRight()));
        info.context->setStrokeGradient(createLinearGradient(regularTopOutline, regularBottomOutline, rect.topRight(), rect.bottomRight()));
    }

    ControlPart part = object->style()->appearance();
    switch (part) {
    case CheckboxPart:
            {
                FloatSize smallCorner(smallRadius, smallRadius);
                info.context->beginPath();
                info.context->addPath(Path::createRoundedRectangle(rect, smallCorner));
                info.context->drawPath();

                if (isChecked(object)) {
                    Path checkPath;
                    IntRect rect2 = rect;
                    rect2.inflate(-1);
                    checkPath.moveTo(FloatPoint(rect2.x() + rect2.width() * checkboxLeftX, rect2.y() + rect2.height() * checkboxLeftY));
                    checkPath.addLineTo(FloatPoint(rect2.x() + rect2.width() * checkboxMiddleX, rect2.bottom() - rect2.height() * checkboxMiddleY));
                    checkPath.addLineTo(FloatPoint(rect2.x() + rect2.width() * checkboxRightX, rect2.y() + rect2.height() * checkboxRightY));
                    info.context->setLineCap(RoundCap);
                    info.context->setStrokeColor(blackPen, DeviceColorSpace);
                    info.context->setStrokeThickness(rect2.width() / checkboxStrokeThickness);
                    info.context->beginPath();
                    info.context->addPath(checkPath);
                    info.context->drawPath();
                }
            }
            break;
    case RadioPart:
            info.context->drawEllipse(rect);
            if (isChecked(object)) {
                IntRect rect2 = rect;
                rect2.inflate(-rect.width() * radioButtonCheckStateScaler);
                info.context->setFillColor(check, DeviceColorSpace);
                info.context->setStrokeColor(check, DeviceColorSpace);
                info.context->drawEllipse(rect2);
            }
            break;
    case ButtonPart:
    case PushButtonPart:
            {
                FloatSize largeCorner(largeRadius, largeRadius);
                info.context->beginPath();
                info.context->addPath(Path::createRoundedRectangle(rect, largeCorner));
                info.context->drawPath();

                break;
            }
    default:
            info.context->restore();
            return true;
    }

    info.context->restore();
    return false;
}

void RenderThemeBlackBerry::adjustMenuListStyle(CSSStyleSelector* css, RenderStyle* style, Element* element) const
{
    adjustMenuListButtonStyle(css, style, element);
}

void RenderThemeBlackBerry::adjustCheckboxStyle(CSSStyleSelector*, RenderStyle* style, Element*) const
{
    setCheckboxSize(style);
    style->setBoxShadow(0);
    Length margin(marginSize, Fixed);
    style->setMarginBottom(margin);
    style->setMarginRight(margin);
}

void RenderThemeBlackBerry::adjustRadioStyle(CSSStyleSelector*, RenderStyle* style, Element*) const
{
    setRadioSize(style);
    style->setBoxShadow(0);
    Length margin(marginSize, Fixed);
    style->setMarginBottom(margin);
    style->setMarginRight(margin);
}

void RenderThemeBlackBerry::paintMenuListButtonGradientAndArrow(GraphicsContext* context, RenderObject* object, IntRect buttonRect, const Path& clipPath)
{
    ASSERT(context);
    context->save();
    if (!isEnabled(object))
        context->setFillGradient(createLinearGradient(disabledTop, disabledBottom, buttonRect.topRight(), buttonRect.bottomRight()));
    else if (isPressed(object))
        context->setFillGradient(createLinearGradient(depressedTop, depressedBottom, buttonRect.topRight(), buttonRect.bottomRight()));
    else if (isHovered(object))
        context->setFillGradient(createLinearGradient(hoverTop, hoverBottom, buttonRect.topRight(), buttonRect.bottomRight()));
    else
        context->setFillGradient(createLinearGradient(regularTop, regularBottom, buttonRect.topRight(),buttonRect.bottomRight()));

    // 1. Paint the background of the button that will contain the arrow.
    context->clip(clipPath);
    context->drawRect(buttonRect);
    context->restore();

    // 2. Paint the button arrow.
    buttonRect.inflate(-buttonRect.width() / 3);
    buttonRect.move(0, buttonRect.height() * 7 / 20);
    Path path;
    path.moveTo(FloatPoint(buttonRect.x(), buttonRect.y()));
    path.addLineTo(FloatPoint(buttonRect.x() + buttonRect.width(), buttonRect.y()));
    path.addLineTo(FloatPoint(buttonRect.x() + buttonRect.width() / 2.0, buttonRect.y() + buttonRect.height() / 2.0));
    path.closeSubpath();

    context->save();
    context->setStrokeStyle(SolidStroke);
    context->setStrokeThickness(lineWidth);
    context->setStrokeColor(Color::black, DeviceColorSpace);
    context->setFillColor(Color::black, DeviceColorSpace);
    context->setLineJoin(BevelJoin);
    context->beginPath();
    context->addPath(path);
    context->drawPath();
    context->restore();
}

static IntRect computeMenuListArrowButtonRect(const IntRect& rect)
{
    // FIXME: The menu list arrow button should have a minimum and maximum width (to ensure usability) or
    // scale with respect to the font size used in the menu list control or some combination of both.
    return IntRect(IntPoint(rect.right() - rect.height(), rect.y()), IntSize(rect.height(), rect.height()));
}

static void paintMenuListBackground(GraphicsContext* context, const Path& menuListPath, const Color& backgroundColor)
{
    ASSERT(context);
    context->save();
    context->setFillColor(backgroundColor, DeviceColorSpace);
    context->beginPath();
    context->addPath(menuListPath);
    context->fillPath();
    context->restore();
}

bool RenderThemeBlackBerry::paintMenuList(RenderObject* object, const RenderObject::PaintInfo& info, const IntRect& rect)
{
    // Note, this method is not called if the menu list explicitly specifies either a border or background color.
    // Instead, RenderThemeBlackBerry::paintMenuListButton is called. Therefore, when this method is called, we don't
    // have to adjust rect with respect to the border dimensions.

    ASSERT(info.context);
    GraphicsContext* context = info.context;

    Path menuListRoundedRectangle = Path::createRoundedRectangle(rect, FloatSize(largeRadius, largeRadius));

    // 1. Paint the background of the entire control.
    paintMenuListBackground(context, menuListRoundedRectangle, Color::white);

    // 2. Paint the background of the button and its arrow.
    IntRect arrowButtonRectangle = computeMenuListArrowButtonRect(rect);
    paintMenuListButtonGradientAndArrow(context, object, arrowButtonRectangle, menuListRoundedRectangle);

    // 4. Stroke an outline around the entire control.
    context->save();
    context->setStrokeStyle(SolidStroke);
    context->setStrokeThickness(lineWidth);
    if (!isEnabled(object))
        context->setStrokeColor(disabledOutline, DeviceColorSpace);
    else if (isPressed(object))
        context->setStrokeGradient(createLinearGradient(depressedTopOutline, depressedBottomOutline, rect.topRight(), rect.bottomRight()));
    else if (isHovered(object))
        context->setStrokeGradient(createLinearGradient(hoverTopOutline, hoverBottomOutline, rect.topRight(), rect.bottomRight()));
    else
        context->setStrokeGradient(createLinearGradient(regularTopOutline, regularBottomOutline, rect.topRight(), rect.bottomRight()));

    context->beginPath();
    context->addPath(menuListRoundedRectangle);
    context->strokePath();
    context->restore();
    return false;
}

bool RenderThemeBlackBerry::paintMenuListButton(RenderObject* object, const RenderObject::PaintInfo& info, const IntRect& rect)
{
    // Note, this method is only called if the menu list explicitly specifies either a border or background color.
    // Otherwise, RenderThemeBlackBerry::paintMenuList is called. We need to fit the arrow button with the border box
    // of the menu-list so as to not occlude the custom border.

    // We compute menuListRoundedRectangle with respect to the dimensions of the entire menu-list control (i.e. rect) and
    // its border radius so that we clip the contour of the arrow button (when we paint it below) to match the contour of
    // the control.
    IntSize topLeftRadius;
    IntSize topRightRadius;
    IntSize bottomLeftRadius;
    IntSize bottomRightRadius;
    object->style()->getBorderRadiiForRect(rect, topLeftRadius, topRightRadius, bottomLeftRadius, bottomRightRadius);
    Path menuListRoundedRectangle = Path::createRoundedRectangle(rect, topLeftRadius, topRightRadius, bottomLeftRadius, bottomRightRadius);

    // 1. Paint the background of the entire control.
    Color fillColor = object->style()->visitedDependentColor(CSSPropertyBackgroundColor);
    if (!fillColor.isValid())
        fillColor = Color::white;
    paintMenuListBackground(info.context, menuListRoundedRectangle, fillColor);

    // 2. Paint the background of the button and its arrow.
    IntRect bounds = IntRect(rect.x() + object->style()->borderLeftWidth(),
                         rect.y() + object->style()->borderTopWidth(),
                         rect.width() - object->style()->borderLeftWidth() - object->style()->borderRightWidth(),
                         rect.height() - object->style()->borderTopWidth() - object->style()->borderBottomWidth());

    IntRect arrowButtonRectangle = computeMenuListArrowButtonRect(bounds); // Fit the arrow button within the border box of the menu-list.
    paintMenuListButtonGradientAndArrow(info.context, object, arrowButtonRectangle, menuListRoundedRectangle);
    return false;
}

void RenderThemeBlackBerry::adjustSliderThumbSize(RenderObject* o) const
{
    ControlPart part = o->style()->appearance();
    if (part == SliderThumbHorizontalPart || part == SliderThumbVerticalPart) {
        o->style()->setWidth(Length(sliderThumbWidth, Fixed));
        o->style()->setHeight(Length(sliderThumbHeight, Fixed));
    }
}

bool RenderThemeBlackBerry::paintSliderTrack(RenderObject* object, const RenderObject::PaintInfo& info, const IntRect& rect)
{
    IntRect rect2 = rect;
    FloatSize smallCorner(smallRadius, smallRadius);
    rect2.setHeight(5);
    rect2.setY(rect2.y() + 10);

    info.context->save();
    info.context->setStrokeStyle(SolidStroke);
    info.context->setStrokeThickness(lineWidth);

    if (isPressed(object) || isHovered(object)) {
        info.context->setStrokeGradient(createLinearGradient(rangeSliderRollTopOutline, rangeSliderRollBottomOutline, rect2.topRight(), rect2. bottomRight()));
        info.context->setFillGradient(createLinearGradient(rangeSliderRollTop, rangeSliderRollBottom, rect2.topRight(), rect2.bottomRight()));
    } else {
        info.context->setStrokeGradient(createLinearGradient(rangeSliderRegularTopOutline, rangeSliderRegularBottomOutline, rect2.topRight(), rect2. bottomRight()));
        info.context->setFillGradient(createLinearGradient(rangeSliderRegularTop, rangeSliderRegularBottom, rect2.topRight(), rect2.bottomRight()));
    }

    info.context->beginPath();
    info.context->addPath(Path::createRoundedRectangle(rect2, smallCorner));
    info.context->drawPath();

    info.context->restore();
    return false;
}

bool RenderThemeBlackBerry::paintSliderThumb(RenderObject* object, const RenderObject::PaintInfo& info, const IntRect& rect)
{
    FloatSize largeCorner(largeRadius, largeRadius);

    info.context->save();
    info.context->setStrokeStyle(SolidStroke);
    info.context->setStrokeThickness(lineWidth);

    if (isPressed(object) || isHovered(object)) {
        info.context->setStrokeGradient(createLinearGradient(hoverTopOutline, hoverBottomOutline, rect.topRight(), rect. bottomRight()));
        info.context->setFillGradient(createLinearGradient(hoverTop, hoverBottom, rect.topRight(), rect.bottomRight()));
    } else {
        info.context->setStrokeGradient(createLinearGradient(regularTopOutline, regularBottomOutline, rect.topRight(), rect. bottomRight()));
        info.context->setFillGradient(createLinearGradient(regularTop, regularBottom, rect.topRight(), rect.bottomRight()));
    }

    info.context->beginPath();
    info.context->addPath(Path::createRoundedRectangle(rect, largeCorner));
    info.context->drawPath();

    IntPoint startPoint(rect.x() + 4, rect.y() + 5);
    IntPoint endPoint(rect.x() + 4, rect.y() + 20);

    for (int i =0; i < 3; i++, startPoint.setX(startPoint.x() + 2), endPoint.setX(endPoint.x() + 2)) {
        if (isPressed(object) || isHovered(object))
            info.context->setStrokeColor(dragRollLight, DeviceColorSpace);
        else
            info.context->setStrokeColor(dragRegularLight, DeviceColorSpace);
        info.context->drawLine(startPoint, endPoint);

        startPoint.setX(startPoint.x() + 1);
        endPoint.setX(endPoint.x() + 1);
        if (isPressed(object) || isHovered(object))
            info.context->setStrokeColor(dragRollDark, DeviceColorSpace);
        else
            info.context->setStrokeColor(dragRegularDark, DeviceColorSpace);
        info.context->drawLine(startPoint, endPoint);
    }
    info.context->restore();
    return false;
}

Color RenderThemeBlackBerry::platformFocusRingColor() const
{
    return Color(outlinePen);
}

Color RenderThemeBlackBerry::platformActiveSelectionBackgroundColor() const
{
    return Color(selection);
}

}
