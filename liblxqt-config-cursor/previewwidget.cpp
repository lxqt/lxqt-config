/* Copyright © 2003-2007 Fredrik Höglund <fredrik@kde.org>
 * (c)GPL2 (c)GPL3
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
/*
 * additional code: Ketmar // Vampire Avalon (psyc://ketmar.no-ip.org/~Ketmar)
 */
#include <QDebug>

#include <QPainter>
#include <QMouseEvent>

#include "previewwidget.h"

#include "crtheme.h"

#include <X11/Xlib.h>
#include <X11/Xcursor/Xcursor.h>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QX11Info>
#include <QWindow>
#endif

namespace {
    // Preview cursors
    const char *const cursorNames[] = {
        "left_ptr",
        "left_ptr_watch",
        "wait",
        "pointing_hand",
        "whats_this",
        "ibeam",
        "size_all",
        "size_fdiag",
        "cross",
        "split_h",
        "size_ver",
        "size_hor",
        "size_bdiag",
        "split_v",
    };

    const int numCursors = 9;       // The number of cursors from the above list to be previewed
    const int previewSize = 24;     // The nominal cursor size to be used in the preview widget
    const int cursorSpacing = 20;   // Spacing between preview cursors
    const int widgetMinWidth = 10;  // The minimum width of the preview widget
    const int widgetMinHeight = 48; // The minimum height of the preview widget
}


///////////////////////////////////////////////////////////////////////////////
class PreviewCursor
{
    public:
    PreviewCursor (const XCursorThemeData &theme, const QString &name);
    ~PreviewCursor () {}

    const QPixmap &pixmap () const { return mPixmap; }
    int width () const { return mPixmap.width(); }
    int height () const { return mPixmap.height(); }
    inline QRect rect () const;
    void setPosition (const QPoint &p) { mPos = p; }
    void setPosition (int x, int y) { mPos = QPoint(x, y); }
    QPoint position () const { return mPos; }
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    operator const xcb_cursor_t& () const { return mCursorHandle; }
#else
    operator const QCursor& () const { return mCursor; }
#endif
    operator const QPixmap& () const { return pixmap(); }

    private:
    QPixmap mPixmap;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    xcb_cursor_t mCursorHandle;
#else
    QCursor mCursor;
#endif
    QPoint mPos;
};


///////////////////////////////////////////////////////////////////////////////
PreviewCursor::PreviewCursor(const XCursorThemeData &theme, const QString &name)
{
    // Create the preview pixmap
    QImage image = theme.loadImage(name, previewSize);
    if (image.isNull()) return;
    int maxSize = previewSize*2;
    if (image.height() > maxSize || image.width() > maxSize)
    image = image.scaled(maxSize, maxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    mPixmap = QPixmap::fromImage(image);
    // load the cursor
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	mCursorHandle = theme.loadCursorHandle(name, previewSize);
#else
    mCursor = theme.loadCursor(name, previewSize);
#endif
}

QRect PreviewCursor::rect() const
{
    return QRect(mPos, mPixmap.size()).adjusted(-(cursorSpacing/2), -(cursorSpacing/2), cursorSpacing/2, cursorSpacing/2);
}


///////////////////////////////////////////////////////////////////////////////
PreviewWidget::PreviewWidget(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);
    mCurrent = NULL;
}

PreviewWidget::~PreviewWidget()
{
    qDeleteAll(mList);
    mList.clear();
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)

// Since Qt5, wrapping a Cursor handle with QCursor is no longer supported.
// So we have to do it ourselves. I really hate Qt 5!
void PreviewWidget::setCursorHandle(xcb_cursor_t cursorHandle)
{
    WId wid = nativeParentWidget()->windowHandle()->winId();
    xcb_change_window_attributes(QX11Info::connection(), wid, XCB_CW_CURSOR, &cursorHandle);
    xcb_flush(QX11Info::connection());
}

#endif


QSize PreviewWidget::sizeHint() const
{
    int totalWidth = 0, maxHeight = 0;
    foreach (const PreviewCursor *c, mList)
    {
        totalWidth += c->width();
        maxHeight = qMax(c->height(), maxHeight);
    }
    totalWidth += (mList.count()-1)*cursorSpacing;
    maxHeight = qMax(maxHeight, widgetMinHeight);
    return QSize(qMax(totalWidth, widgetMinWidth), qMax(height(), maxHeight));
}

void PreviewWidget::layoutItems()
{
    if (!mList.isEmpty())
    {
        QSize size = sizeHint();
        int cursorWidth = size.width()/mList.count();
        int nextX = (width()-size.width())/2;
        foreach (PreviewCursor *c, mList)
        {
            c->setPosition(nextX+(cursorWidth-c->width())/2, (height()-c->height())/2);
            nextX += cursorWidth;
        }
    }
    mNeedLayout = false;
}

void PreviewWidget::setTheme(const XCursorThemeData &theme)
{
    qDeleteAll(mList);
    mList.clear();
    for (int i = 0; i < numCursors; ++i) mList << new PreviewCursor(theme, cursorNames[i]);
    mNeedLayout = true;
    updateGeometry();
    mCurrent = NULL;
    update();
}


void PreviewWidget::clearTheme()
{
    qDeleteAll(mList);
    mList.clear();
    mCurrent = NULL;
    update();
}

void PreviewWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    if (mNeedLayout) layoutItems();
    foreach (const PreviewCursor *c, mList)
    {
        if (c->pixmap().isNull()) continue;
        p.drawPixmap(c->position(), *c);
    }
}

void PreviewWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (mNeedLayout) layoutItems();
    foreach (const PreviewCursor *c, mList)
    {
        if (c->rect().contains(e->pos()))
        {
            if (c != mCurrent)
            {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                // NOTE: we have to set the Qt cursor value to something other than Qt::ArrowCursor
                // Otherwise, though we set the xcursor handle to the underlying window, Qt still
                // thinks that the current cursor is Qt::ArrowCursor and will not restore the cursor
                // later when we call setCursor(Qt::ArrowCursor). So, we set it to BlankCursor to
                // cheat Qt so it knows that the current cursor is not Qt::ArrowCursor.
                // This is a dirty hack, but without this, Qt cannot restore Qt::ArrowCursor later.
                setCursor(Qt::BlankCursor);
                setCursorHandle(*c);
#else
                setCursor(*c);
#endif
                mCurrent = c;
            }
            return;
        }
    }
    setCursor(Qt::ArrowCursor);
    mCurrent = NULL;
}


void PreviewWidget::resizeEvent(QResizeEvent *)
{
    if (!mList.isEmpty()) mNeedLayout = true;
}
