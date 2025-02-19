/************************************************************************
 **
 **  @file   def.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 4, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013 - 2022 Seamly2D project
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

#ifndef DEF_H
#define DEF_H

#include <qcompilerdetection.h>
#include <QFileDialog>
#include <QLineF>
#include <QString>
#include <QStringList>
#include <Qt>
#include <QtGlobal>
#include <QPrinter>
#include <csignal>
#ifdef Q_OS_WIN
    #include <windows.h>
#endif /* Q_OS_WIN */

#include "debugbreak.h"

template <class T> class QSharedPointer;

#ifdef Q_CC_MSVC
    #include <ciso646>
#endif /* Q_CC_MSVC */

class QComboBox;
class QMarginsF;
class VTranslateMeasurements;
class QGraphicsItem;

#define SceneSize 50000

enum class PaperSizeFormat : char { A0 = 0,
                                    A1,
                                    A2,
                                    A3,
                                    A4,
                                    Letter,
                                    Legal,
                                    Tabloid,
                                    AnsiC,
                                    AnsiD,
                                    AnsiE,
                                    Roll24in,     // Be careful when changing order roll type
                                    Roll30in,     // Used also for showing icon
                                    Roll36in,
                                    Roll42in,
                                    Roll44in,
                                    Custom
};

enum class LayoutExportFormat : char
{
    SVG = 0,
    PDF = 1,
    PDFTiled = 2,
    PNG = 3,
    JPG = 4,
    BMP = 5,
    PPM = 6,
    OBJ = 7,              /* Wavefront OBJ*/
    PS  = 8,
    EPS = 9,
    DXF_AC1006_Flat = 10,  /* R10. */
    DXF_AC1009_Flat = 11,  /* R11 & R12. */
    DXF_AC1012_Flat = 12,  /* R13. */
    DXF_AC1014_Flat = 13,  /* R14. */
    DXF_AC1015_Flat = 14, /* ACAD 2000. */
    DXF_AC1018_Flat = 15, /* ACAD 2004. */
    DXF_AC1021_Flat = 16, /* ACAD 2007. */
    DXF_AC1024_Flat = 17, /* ACAD 2010. */
    DXF_AC1027_Flat = 18, /* ACAD 2013. */
    DXF_AC1006_AAMA = 19, /* R10. */
    DXF_AC1009_AAMA = 20, /* R11 & R12. */
    DXF_AC1012_AAMA = 21, /* R13. */
    DXF_AC1014_AAMA = 22, /* R14. */
    DXF_AC1015_AAMA = 23, /* ACAD 2000. */
    DXF_AC1018_AAMA = 24, /* ACAD 2004. */
    DXF_AC1021_AAMA = 25, /* ACAD 2007. */
    DXF_AC1024_AAMA = 26, /* ACAD 2010. */
    DXF_AC1027_AAMA = 27, /* ACAD 2013. */
    DXF_AC1006_ASTM = 28, /* R10. */
    DXF_AC1009_ASTM = 29, /* R11 & R12. */
    DXF_AC1012_ASTM = 30, /* R13. */
    DXF_AC1014_ASTM = 31, /* R14. */
    DXF_AC1015_ASTM = 32, /* ACAD 2000. */
    DXF_AC1018_ASTM = 33, /* ACAD 2004. */
    DXF_AC1021_ASTM = 34, /* ACAD 2007. */
    DXF_AC1024_ASTM = 35, /* ACAD 2010. */
    DXF_AC1027_ASTM = 36, /* ACAD 2013. */
    TIF = 37,             /* TIFF */
    COUNT                 /*Use only for validation*/
};

enum class NodeDetail : char { Contour, Modeling };
enum class SceneObject : char { Point, Line, Spline, Arc, ElArc, SplinePath, Piece, Unknown };
enum class MeasurementsType : char { Multisize, Individual , Unknown};
enum class Unit : char { Mm = 0, Cm, Inch, Px, LAST_UNIT_DO_NOT_USE};
enum class Source : char { FromGui, FromFile, FromTool };
enum class NodeUsage : bool {NotInUse = false, InUse = true};
enum class SelectionType : bool {ByMousePress, ByMouseRelease};

enum class PageOrientation : bool {Portrait = true, Landscape = false};

enum class PieceNodeAngle : unsigned char
{
    ByLength = 0,
    ByPointsIntersection,
    ByFirstEdgeSymmetry,
    BySecondEdgeSymmetry,
    ByFirstEdgeRightAngle,
    BySecondEdgeRightAngle
};

enum class NotchType : unsigned char
{
    Slit = 0, // Default
    TNotch,
    VInternal,
    VExternal,
    UNotch,
    Castle,
    Diamond
};

QString      notchTypeToString(NotchType type);
NotchType    stringToNotchType(const QString &value);

enum class NotchSubType : unsigned char
{
    Straightforward = 0, // Default
    Bisector,
    Intersection
};

QString      notchSubTypeToString(NotchSubType type);
NotchSubType stringToNotchSubType(const QString &value);


Unit         StrToUnits(const QString &unit);
QString      UnitsToStr(const Unit &unit, const bool translate = false);


enum class PiecePathIncludeType : unsigned char
{
    AsMainPath = 0,
    AsCustomSA = 1
};

enum class PiecePathType :  unsigned char {PiecePath = 0, CustomSeamAllowance = 1, InternalPath = 2, Unknown = 3};

typedef int ToolVisHolderType;
enum class Tool : ToolVisHolderType
{
    Arrow,
    SinglePoint,
    DoublePoint,
    LinePoint,
    AbstractSpline,
    Cut,
    BasePoint,
    EndLine,
    Line,
    AlongLine,
    ShoulderPoint,
    Normal,
    Bisector,
    LineIntersect,
    Spline,
    CubicBezier,
    CutSpline,
    CutArc,
    Arc,
    ArcWithLength,
    SplinePath,
    CubicBezierPath,
    CutSplinePath,
    PointOfContact,
    Piece,
    InternalPath,
    NodePoint,
    NodeArc,
    NodeElArc,
    NodeSpline,
    NodeSplinePath,
    Height,
    Triangle,
    LineIntersectAxis,
    PointOfIntersectionArcs,
    PointOfIntersectionCircles,
    PointOfIntersectionCurves,
    CurveIntersectAxis,
    ArcIntersectAxis,
    PointOfIntersection,
    PointFromCircleAndTangent,
    PointFromArcAndTangent,
    TrueDarts,
    Union,
    Group,
    Rotation,
    MirrorByLine,
    MirrorByAxis,
    Move,
    Midpoint,
    EllipticalArc,
    AnchorPoint,
    InsertNodes,
    LAST_ONE_DO_NOT_USE //add new stuffs above this, this constant must be last and never used
};

enum class Vis : ToolVisHolderType
{
    ControlPointSpline = static_cast<ToolVisHolderType>(Tool::LAST_ONE_DO_NOT_USE),
    GraphicsSimpleTextItem,
    SimplePoint,
    SimpleCurve,
    ScaledLine,
    ScaledEllipse,
    Line,
    Path,
    Operation,
    ToolAlongLine,
    ToolArc,
    ToolArcWithLength,
    ToolBisector,
    ToolCutArc,
    ToolEndLine,
    ToolHeight,
    ToolLine,
    ToolLineIntersect,
    ToolNormal,
    ToolPointOfContact,
    ToolPointOfIntersection,
    ToolPointOfIntersectionArcs,
    ToolPointOfIntersectionCircles,
    ToolPointOfIntersectionCurves,
    ToolPointFromCircleAndTangent,
    ToolPointFromArcAndTangent,
    ToolShoulderPoint,
    ToolSpline,
    ToolCubicBezier,
    ToolCubicBezierPath,
    ToolTriangle,
    ToolCutSpline,
    ToolSplinePath,
    ToolCutSplinePath,
    ToolLineIntersectAxis,
    ToolCurveIntersectAxis,
    ToolTrueDarts,
    ToolRotation,
    ToolMirrorByLine,
    ToolMirrorByAxis,
    ToolMove,
    ToolEllipticalArc,
    ToolPiece,
    ToolInternalPath,
    ToolAnchorPoint,
    PieceAnchors,
    NoBrush,
    CurvePathItem,
    GrainlineItem,
    PieceItem,
    TextGraphicsItem,
    ScenePoint,
    ArrowedLineItem,
    LAST_ONE_DO_NOT_USE //add new types above this, this constant must be last and never used
};

enum class VarType : char { Measurement, Increment, LineLength, CurveLength, CurveCLength, LineAngle, CurveAngle,
                            ArcRadius, Unknown };

static const int heightStep = 6;
enum class GHeights : unsigned char { ALL,
                                      H50=50,   H56=56,   H62=62,   H68=68,   H74=74,   H80=80,   H86=86,   H92=92,
                                      H98=98,   H104=104, H110=110, H116=116, H122=122, H128=128, H134=134, H140=140,
                                      H146=146, H152=152, H158=158, H164=164, H170=170, H176=176, H182=182, H188=188,
                                      H194=194, H200=200};

static const int sizeStep = 2;
enum class GSizes : unsigned char { ALL,
                                    S22=22, S24=24, S26=26, S28=28, S30=30, S32=32, S34=34, S36=36, S38=38, S40=40,
                                    S42=42, S44=44, S46=46, S48=48, S50=50, S52=52, S54=54, S56=56, S58=58, S60=60,
                                    S62=62, S64=64, S66=66, S68=68, S70=70, S72=72 };

/* QImage supports a maximum of 32768x32768 px images (signed short).
 * This follows from the condition: width * height * colordepth < INT_MAX (4 billion) -> 32768 * 32768 * 4 = 4 billion.
 * The second condition is of course that malloc is able to allocate the requested memory.
 *
 * If you really need bigger images you will have to use another wrapper or split into multiple QImage's.
 */
#define QIMAGE_MAX 32768

/*
 * This macros SCASSERT (for Stop and Continue Assert) will break into the debugger on the line of the assert and allow
 * you to continue afterwards should you choose to.
 * idea: Q_ASSERT no longer pauses debugger - http://qt-project.org/forums/viewthread/13148
 * Usefull links:
 * 1. What's the difference between __PRETTY_FUNCTION__, __FUNCTION__, __func__? -
 *    https://stackoverflow.com/questions/4384765/whats-the-difference-between-pretty-function-function-func
 *
 * 2. Windows Predefined Macros - http://msdn.microsoft.com/library/b0084kay.aspx
 *
 * 3. Windows DebugBreak function - http://msdn.microsoft.com/en-us/library/ms679297%28VS.85%29.aspx
 *
 * 4. Continue to debug after failed assertion on Linux? [C/C++] -
 * https://stackoverflow.com/questions/1721543/continue-to-debug-after-failed-assertion-on-linux-c-c
 */
#ifndef V_NO_ASSERT

#define SCASSERT(cond)                                      \
{                                                           \
    if (!(cond))                                            \
    {                                                       \
        qCritical("ASSERT: %s in %s (%s:%u)",               \
                  #cond, Q_FUNC_INFO , __FILE__, __LINE__); \
        debug_break();                                      \
        abort();                                            \
    }                                                       \
}                                                           \

#else // define but disable this function if debugging is not set
#define SCASSERT(cond) qt_noop();
#endif /* V_NO_ASSERT */

#ifndef __has_cpp_attribute
# define __has_cpp_attribute(x) 0
#endif

#if __cplusplus > 201402L && __has_cpp_attribute(fallthrough)
#   define V_FALLTHROUGH [[fallthrough]];
#elif defined(Q_CC_CLANG) && __cplusplus >= 201103L
    /* clang's fallthrough annotations are only available starting in C++11. */
#   define V_FALLTHROUGH [[clang::fallthrough]];
#elif defined(Q_CC_MSVC)
   /*
    * MSVC's __fallthrough annotations are checked by /analyze (Code Analysis):
    * https://msdn.microsoft.com/en-us/library/ms235402%28VS.80%29.aspx
    */
#   include <sal.h>
#   define V_FALLTHROUGH __fallthrough;
#elif defined(Q_CC_GNU) && (__GNUC__ >= 7)
#   define V_FALLTHROUGH [[gnu::fallthrough]];
#else
#   define V_FALLTHROUGH
#endif

extern const QString LONG_OPTION_NO_HDPI_SCALING;
bool IsOptionSet(int argc, char *argv[], const char *option);
void initHighDpiScaling(int argc, char *argv[]);

// functions
extern const QString degTorad_F;
extern const QString radTodeg_F;
extern const QString sin_F;
extern const QString cos_F;
extern const QString tan_F;
extern const QString asin_F;
extern const QString acos_F;
extern const QString atan_F;
extern const QString sinh_F;
extern const QString cosh_F;
extern const QString tanh_F;
extern const QString asinh_F;
extern const QString acosh_F;
extern const QString atanh_F;
extern const QString sinD_F;
extern const QString cosD_F;
extern const QString tanD_F;
extern const QString asinD_F;
extern const QString acosD_F;
extern const QString atanD_F;
extern const QString log2_F;
extern const QString log10_F;
extern const QString log_F;
extern const QString ln_F;
extern const QString exp_F;
extern const QString sqrt_F;
extern const QString sign_F;
extern const QString rint_F;
extern const QString abs_F;
extern const QString min_F;
extern const QString max_F;
extern const QString sum_F;
extern const QString avg_F;
extern const QString fmod_F;

extern const QStringList builInFunctions;

// Postfix operators
extern const QString cm_Oprt;
extern const QString mm_Oprt;
extern const QString in_Oprt;

extern const QStringList builInPostfixOperators;

// Placeholders
extern const QString pl_size;
extern const QString pl_height;
extern const QString pl_date;
extern const QString pl_time;
extern const QString pl_patternName;
extern const QString pl_patternNumber;
extern const QString pl_author;
extern const QString pl_customer;
extern const QString pl_pExt;
extern const QString pl_pFileName;
extern const QString pl_mFileName;
extern const QString pl_mExt;
extern const QString pl_pLetter;
extern const QString pl_pAnnotation;
extern const QString pl_pOrientation;
extern const QString pl_pRotation;
extern const QString pl_pTilt;
extern const QString pl_pFoldPosition;
extern const QString pl_pName;
extern const QString pl_pQuantity;
extern const QString pl_mFabric;
extern const QString pl_mLining;
extern const QString pl_mInterfacing;
extern const QString pl_mInterlining;
extern const QString pl_wCut;
extern const QString pl_wOnFold;

extern const QStringList labelTemplatePlaceholders;

extern const QString cursorArrowOpenHand;
extern const QString cursorArrowCloseHand;

extern const QString degreeSymbol;
extern const QString trueStr;
extern const QString falseStr;

extern const QString strSlit;

extern const QString strStraightforward;
extern const QString strBisector;
extern const QString strIntersection;

extern const QString unitMM;
extern const QString unitCM;
extern const QString unitINCH;
extern const QString unitPX;

extern const QString valExt;
extern const QString vitExt;
extern const QString vstExt;
extern const QString sm2dExt;
extern const QString smisExt;
extern const QString smmsExt;

void SetItemOverrideCursor(QGraphicsItem *item, const QString & pixmapPath, int hotX = -1, int hotY = -1);

extern const qreal PrintDPI;

Q_REQUIRED_RESULT double ToPixel(double val, const Unit &unit);
Q_REQUIRED_RESULT double FromPixel(double pix, const Unit &unit);

Q_REQUIRED_RESULT qreal UnitConvertor(qreal value, const Unit &from, const Unit &to);
Q_REQUIRED_RESULT QMarginsF UnitConvertor(const QMarginsF &margins, const Unit &from, const Unit &to);

void InitLanguages(QComboBox *combobox);
Q_REQUIRED_RESULT QStringList SupportedLocales();

QString makeHeaderName(const QString &name);
Q_REQUIRED_RESULT QString strippedName(const QString &fullFileName);
Q_REQUIRED_RESULT QString RelativeMPath(const QString &patternPath, const QString &absoluteMPath);
Q_REQUIRED_RESULT QString AbsoluteMPath(const QString &patternPath, const QString &relativeMPath);
Q_REQUIRED_RESULT QString fileDialog(QWidget *parent, const QString &title,  const QString &dir,
                                     const QString &filter, QString *selectedFilter, QFileDialog::Options options,
                                     QFileDialog::FileMode mode,  QFileDialog::AcceptMode accept);

Q_REQUIRED_RESULT QSharedPointer<QPrinter> PreparePrinter(const QPrinterInfo &info,
                                                          QPrinter::PrinterMode mode = QPrinter::ScreenResolution);

QMarginsF GetMinPrinterFields(const QSharedPointer<QPrinter> &printer);
QMarginsF GetPrinterFields(const QSharedPointer<QPrinter> &printer);

Q_REQUIRED_RESULT QPixmap darkenPixmap(const QPixmap &pixmap);

void ShowInGraphicalShell(const QString &filePath);

constexpr qreal accuracyPointOnLine = (0.1555/*mm*/ / 25.4) * 96.0;

Q_REQUIRED_RESULT static inline bool VFuzzyComparePoints(const QPointF &p1, const QPointF &p2,
                                                         qreal accuracy = accuracyPointOnLine);

static inline bool VFuzzyComparePoints(const QPointF &p1, const QPointF &p2, qreal accuracy)
{
    return QLineF(p1, p2).length() <= accuracy;
}

Q_REQUIRED_RESULT static inline bool VFuzzyComparePossibleNulls(double p1, double p2);
static inline bool VFuzzyComparePossibleNulls(double p1, double p2)
{
    if(qFuzzyIsNull(p1))
    {
        return qFuzzyIsNull(p2);
    }
    else if(qFuzzyIsNull(p2))
    {
        return false;
    }
    else
    {
        return qFuzzyCompare(p1, p2);
    }
}

/**
 * @brief The CustomSA struct contains record about custom seam allowanse (SA).
 */
struct CustomSARecord
{
    CustomSARecord()
        : startPoint(0),
          path(0),
          endPoint(0),
          reverse(false),
          includeType(PiecePathIncludeType::AsCustomSA)
    {}

    quint32 startPoint;
    quint32 path;
    quint32 endPoint;
    bool reverse;
    PiecePathIncludeType includeType;
};

Q_DECLARE_METATYPE(CustomSARecord)
Q_DECLARE_TYPEINFO(CustomSARecord, Q_MOVABLE_TYPE);

/****************************************************************************
** This file is derived from code bearing the following notice:
** The sole author of this file, Adam Higerd, has explicitly disclaimed all
** copyright interest and protection for the content within. This file has
** been placed in the public domain according to United States copyright
** statute and case law. In jurisdictions where this public domain dedication
** is not legally recognized, anyone who receives a copy of this file is
** permitted to use, modify, duplicate, and redistribute this file, in whole
** or in part, with no restrictions or conditions. In these jurisdictions,
** this file shall be copyright (C) 2006-2008 by Adam Higerd.
****************************************************************************/

#define QXT_DECLARE_PRIVATE(PUB) friend class PUB##Private; QxtPrivateInterface<PUB, PUB##Private> qxt_d;
#define QXT_DECLARE_PUBLIC(PUB) friend class PUB;
#define QXT_INIT_PRIVATE(PUB) qxt_d.setPublic(this);
#define QXT_D(PUB) PUB##Private& d = qxt_d()
#define QXT_P(PUB) PUB& p = qxt_p()

template <typename PUB>
class QxtPrivate
{
public:
    QxtPrivate(): qxt_p_ptr(nullptr)
    {}
    virtual ~QxtPrivate()
    {}
    inline void QXT_setPublic(PUB* pub)
    {
        qxt_p_ptr = pub;
    }

protected:
    inline PUB& qxt_p()
    {
        return *qxt_p_ptr;
    }
    inline const PUB& qxt_p() const
    {
        return *qxt_p_ptr;
    }
    inline PUB* qxt_ptr()
    {
        return qxt_p_ptr;
    }
    inline const PUB* qxt_ptr() const
    {
        return qxt_p_ptr;
    }

private:
    Q_DISABLE_COPY(QxtPrivate)
    PUB* qxt_p_ptr;
};

template <typename PUB, typename PVT>
class QxtPrivateInterface
{
    friend class QxtPrivate<PUB>;
public:
    QxtPrivateInterface() : pvt(new PVT)
    {}
    ~QxtPrivateInterface()
    {
        delete pvt;
    }

    inline void setPublic(PUB* pub)
    {
        pvt->QXT_setPublic(pub);
    }
    inline PVT& operator()()
    {
        return *static_cast<PVT*>(pvt);
    }
    inline const PVT& operator()() const
    {
        return *static_cast<PVT*>(pvt);
    }
    inline PVT * operator->()
    {
    return static_cast<PVT*>(pvt);
    }
    inline const PVT * operator->() const
    {
    return static_cast<PVT*>(pvt);
    }
private:
    Q_DISABLE_COPY(QxtPrivateInterface)
    QxtPrivate<PUB>* pvt;
};

/*
    Convert to a QSet
*/
//---------------------------------------------------------------------------------------------------------------------
template <typename T, template <typename> class C>
inline QSet<T> convertToSet(const C<T> &list)
{
    return QSet<T>(list.begin(), list.end());
}

/*
    Convert to a QSet
*/
//---------------------------------------------------------------------------------------------------------------------
template <typename T, typename C>
inline QSet<T> convertToSet(const C &list)
{
    return QSet<T>(list.begin(), list.end());
}

/*
    Convert to a QList
*/
//---------------------------------------------------------------------------------------------------------------------
template <typename T, template <typename> class C>
inline QList<T> convertToList(const C<T> &set)
{
    return QList<T>(set.begin(), set.end());
}


#endif // DEF_H
