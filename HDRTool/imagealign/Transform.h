#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include "Matrix.h"
/*#include <QtGui/qmatrix.h>
#include <QtGui/qpainterpath.h>
#include <QtGui/qpolygon.h>
#include <QtGui/qregion.h>
#include <QtGui/qwindowdefs.h>
#include <QtCore/qline.h>
#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>

#if defined(Q_OS_VXWORKS) && defined(m_type)
#  undef m_type
#endif

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QVariant;
*/
class /*Q_GUI_EXPORT*/ Transform
{
public:
    enum TransformationType {
        TxNone      = 0x00,
        TxTranslate = 0x01,
        TxScale     = 0x02,
        TxRotate    = 0x04,
        TxShear     = 0x08,
        TxProject   = 0x10
    };

    //inline explicit QTransform(Qt::Initialization) : affine(Qt::Uninitialized) {}
    Transform();
    Transform(float h11, float h12, float h13,
               float h21, float h22, float h23,
               float h31, float h32, float h33 = 1.0);
    Transform(float h11, float h12, float h21,
               float h22, float dx, float dy);
    explicit Transform(const Matrix &mtx);

    bool isAffine() const;
    bool isIdentity() const;
    bool isInvertible() const;
    bool isScaling() const;
    bool isRotating() const;
    bool isTranslating() const;

    TransformationType type() const;

    inline float determinant() const;
    float det() const;

    float m11() const;
    float m12() const;
    float m13() const;
    float m21() const;
    float m22() const;
    float m23() const;
    float m31() const;
    float m32() const;
    float m33() const;
    float dx() const;
    float dy() const;

    void setMatrix(float m11, float m12, float m13,
                   float m21, float m22, float m23,
                   float m31, float m32, float m33);

    //Transform inverted(bool *invertible = 0) const;
    Transform adjoint() const;
    Transform transposed() const;

    Transform &translate(float dx, float dy);
    Transform &scale(float sx, float sy);
    //Transform &shear(float sh, float sv);
    //Transform &rotate(float a, Qt::Axis axis = Qt::ZAxis);
    //Transform &rotateRadians(float a, Qt::Axis axis = Qt::ZAxis);

    //static bool squareToQuad(const QPolygonF &square, Transform &result);
    //static bool quadToSquare(const QPolygonF &quad, Transform &result);
    //static bool quadToQuad(const QPolygonF &one, const QPolygonF &two, Transform &result);

    bool operator==(const Transform &) const;
    bool operator!=(const Transform &) const;

    Transform &operator*=(const Transform &);
    Transform operator*(const Transform &o) const;

    Transform &operator=(const Transform &);

    //operator QVariant() const;


    void reset();
    /*QPoint       map(const QPoint &p) const;
    QPointF      map(const QPointF &p) const;
    QLine        map(const QLine &l) const;
    QLineF       map(const QLineF &l) const;
    QPolygonF    map(const QPolygonF &a) const;
    QPolygon     map(const QPolygon &a) const;
    QRegion      map(const QRegion &r) const;
    QPainterPath map(const QPainterPath &p) const;
    QPolygon     mapToPolygon(const QRect &r) const;
    QRect mapRect(const QRect &) const;
    QRectF mapRect(const QRectF &) const;
    void map(int x, int y, int *tx, int *ty) const;
    void map(qreal x, qreal y, qreal *tx, qreal *ty) const;*/

    const Matrix &toAffine() const;

    Transform &operator*=(float div);
    Transform &operator/=(float div);
    Transform &operator+=(float div);
    Transform &operator-=(float div);

    static Transform fromTranslate(float dx, float dy);
    static Transform fromScale(float dx, float dy);

private:
    inline Transform(float h11, float h12, float h13,
                      float h21, float h22, float h23,
                      float h31, float h32, float h33, bool)
        : affine(h11, h12, h21, h22, h31, h32, true)
        , m_13(h13), m_23(h23), m_33(h33)
        , m_type(TxNone)
        , m_dirty(TxProject) {}
    inline Transform(bool)
        : affine(true)
        , m_13(0), m_23(0), m_33(1)
        , m_type(TxNone)
        , m_dirty(TxNone) {}
    inline TransformationType inline_type() const;
    Matrix affine;
    float   m_13;
    float   m_23;
    float   m_33;

    mutable unsigned int m_type : 5;
    mutable unsigned int m_dirty : 5;

    class Private;
    Private *d;
};
//Q_DECLARE_TYPEINFO(QTransform, Q_MOVABLE_TYPE);

/******* inlines *****/
inline Transform::TransformationType Transform::inline_type() const
{
    if (m_dirty == TxNone)
        return static_cast<TransformationType>(m_type);
    return type();
}

inline bool Transform::isAffine() const
{
    return inline_type() < TxProject;
}
inline bool Transform::isIdentity() const
{
    return inline_type() == TxNone;
}

/*inline bool Transform::isInvertible() const
{
    return !qFuzzyIsNull(determinant());
}*/

inline bool Transform::isScaling() const
{
    return type() >= TxScale;
}
inline bool Transform::isRotating() const
{
    return inline_type() >= TxRotate;
}

inline bool Transform::isTranslating() const
{
    return inline_type() >= TxTranslate;
}

inline float Transform::determinant() const
{
    return affine._m11*(m_33*affine._m22-affine._dy*m_23) -
        affine._m21*(m_33*affine._m12-affine._dy*m_13)+affine._dx*(m_23*affine._m12-affine._m22*m_13);
}
inline float Transform::det() const
{
    return determinant();
}
inline float Transform::m11() const
{
    return affine._m11;
}
inline float Transform::m12() const
{
    return affine._m12;
}
inline float Transform::m13() const
{
    return m_13;
}
inline float Transform::m21() const
{
    return affine._m21;
}
inline float Transform::m22() const
{
    return affine._m22;
}
inline float Transform::m23() const
{
    return m_23;
}
inline float Transform::m31() const
{
    return affine._dx;
}
inline float Transform::m32() const
{
    return affine._dy;
}
inline float Transform::m33() const
{
    return m_33;
}
inline float Transform::dx() const
{
    return affine._dx;
}
inline float Transform::dy() const
{
    return affine._dy;
}

inline Transform &Transform::operator*=(float num)
{
    if (num == 1.)
        return *this;
    affine._m11 *= num;
    affine._m12 *= num;
    m_13        *= num;
    affine._m21 *= num;
    affine._m22 *= num;
    m_23        *= num;
    affine._dx  *= num;
    affine._dy  *= num;
    m_33        *= num;
    if (m_dirty < TxScale)
        m_dirty = TxScale;
    return *this;
}
inline Transform &Transform::operator/=(float div)
{
    if (div == 0)
        return *this;
    div = 1/div;
    return operator*=(div);
}
inline Transform &Transform::operator+=(float num)
{
    if (num == 0)
        return *this;
    affine._m11 += num;
    affine._m12 += num;
    m_13        += num;
    affine._m21 += num;
    affine._m22 += num;
    m_23        += num;
    affine._dx  += num;
    affine._dy  += num;
    m_33        += num;
    m_dirty     = TxProject;
    return *this;
}
inline Transform &Transform::operator-=(float num)
{
    if (num == 0)
        return *this;
    affine._m11 -= num;
    affine._m12 -= num;
    m_13        -= num;
    affine._m21 -= num;
    affine._m22 -= num;
    m_23        -= num;
    affine._dx  -= num;
    affine._dy  -= num;
    m_33        -= num;
    m_dirty     = TxProject;
    return *this;
}

/*inline bool qFuzzyCompare(const Transform& t1, const Transform& t2)
{
    return qFuzzyCompare(t1.m11(), t2.m11())
        && qFuzzyCompare(t1.m12(), t2.m12())
        && qFuzzyCompare(t1.m13(), t2.m13())
        && qFuzzyCompare(t1.m21(), t2.m21())
        && qFuzzyCompare(t1.m22(), t2.m22())
        && qFuzzyCompare(t1.m23(), t2.m23())
        && qFuzzyCompare(t1.m31(), t2.m31())
        && qFuzzyCompare(t1.m32(), t2.m32())
        && qFuzzyCompare(t1.m33(), t2.m33());
}*/


/****** stream functions *******************/
/*#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QTransform &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QTransform &);
#endif*/

/*#ifndef QT_NO_DEBUG_STREAM
Q_GUI_EXPORT QDebug operator<<(QDebug, const QTransform &);
#endif*/
/****** end stream functions *******************/

// mathematical semantics
/*Q_GUI_EXPORT_INLINE QPoint operator*(const QPoint &p, const QTransform &m)
{ return m.map(p); }
Q_GUI_EXPORT_INLINE QPointF operator*(const QPointF &p, const QTransform &m)
{ return m.map(p); }
Q_GUI_EXPORT_INLINE QLineF operator*(const QLineF &l, const QTransform &m)
{ return m.map(l); }
Q_GUI_EXPORT_INLINE QLine operator*(const QLine &l, const QTransform &m)
{ return m.map(l); }
Q_GUI_EXPORT_INLINE QPolygon operator *(const QPolygon &a, const QTransform &m)
{ return m.map(a); }
Q_GUI_EXPORT_INLINE QPolygonF operator *(const QPolygonF &a, const QTransform &m)
{ return m.map(a); }
Q_GUI_EXPORT_INLINE QRegion operator *(const QRegion &r, const QTransform &m)
{ return m.map(r); }
Q_GUI_EXPORT_INLINE QPainterPath operator *(const QPainterPath &p, const QTransform &m)
{ return m.map(p); }*/

/*Q_GUI_EXPORT_INLINE QTransform operator *(const QTransform &a, qreal n)
{ QTransform t(a); t *= n; return t; }
Q_GUI_EXPORT_INLINE QTransform operator /(const QTransform &a, qreal n)
{ QTransform t(a); t /= n; return t; }
Q_GUI_EXPORT_INLINE QTransform operator +(const QTransform &a, qreal n)
{ QTransform t(a); t += n; return t; }
Q_GUI_EXPORT_INLINE QTransform operator -(const QTransform &a, qreal n)
{ QTransform t(a); t -= n; return t; }*/

/*QT_END_NAMESPACE

QT_END_HEADER*/

#endif 