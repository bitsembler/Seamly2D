/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                             *
 *                                                                         *
 ***************************************************************************
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Seamly2D is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 **************************************************************************

 ************************************************************************
 **
 **  @file   vcontour.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 1, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Seamly2D is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#ifndef VCONTOUR_H
#define VCONTOUR_H

#include <QSharedDataPointer>
#include <QSizeF>
#include <QTypeInfo>
#include <QVector>
#include <QtGlobal>

#include "vlayoutdef.h"

class VContourData;
class QPointF;
class QLineF;
class QRectF;
class QPainterPath;
class VLayoutPiece;

class VContour
{
public:
    VContour();
    VContour(int height, int width);
    VContour(const VContour &contour);

    ~VContour();

    VContour &operator=(const VContour &contour);
#ifdef Q_COMPILER_RVALUE_REFS
	VContour &operator=(VContour &&contour) Q_DECL_NOTHROW;
#endif

	void Swap(VContour &contour) Q_DECL_NOTHROW;

    void             SetContour(const QVector<QPointF> &contour);
    QVector<QPointF> GetContour() const;

    quint32 GetShift() const;
    void    SetShift(quint32 shift);

    int  GetHeight() const;
    void setHeight(int height);

    int  GetWidth() const;
    void SetWidth(int width);

    QSizeF GetSize() const;

    QVector<QPointF> UniteWithContour(const VLayoutPiece &detail, int globalI, int detJ, BestFrom type) const;

    QLineF EmptySheetEdge() const;
    int    GlobalEdgesCount() const;
    QLineF GlobalEdge(int i) const;
    QVector<QPointF> CutEdge(const QLineF &edge) const;
    QVector<QPointF> CutEmptySheetEdge() const;

    const QPointF &	at(int i) const;

    QRectF BoundingRect() const;

    QPainterPath ContourPath() const;

private:
    QSharedDataPointer<VContourData> d;

    void AppendWhole(QVector<QPointF> &contour, const VLayoutPiece &detail, int detJ) const;
};

Q_DECLARE_TYPEINFO(VContour, Q_MOVABLE_TYPE);

#endif // VCONTOUR_H
