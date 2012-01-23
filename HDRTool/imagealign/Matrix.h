#ifndef _MATRIX_H_
#define _MATRIX_H_


class Matrix 
{
public:
    //inline explicit Matrix(Qt::Initialization) {}
    Matrix();
    Matrix(float m11, float m12, float m21, float m22,
            float dx, float dy);
    Matrix(const Matrix &matrix);

    void setMatrix(float m11, float m12, float m21, float m22,
                   float dx, float dy);

    float m11() const { return _m11; }
    float m12() const { return _m12; }
    float m21() const { return _m21; }
    float m22() const { return _m22; }
    float dx() const { return _dx; }
    float dy() const { return _dy; }

    //void map(int x, int y, int *tx, int *ty) const;
    //void map(qreal x, qreal y, qreal *tx, qreal *ty) const;
    //QRect mapRect(const QRect &) const;
    //QRectF mapRect(const QRectF &) const;

    //QPoint map(const QPoint &p) const;
    //QPointF map(const QPointF&p) const;
    //QLine map(const QLine &l) const;
    //QLineF map(const QLineF &l) const;
    //QPolygonF map(const QPolygonF &a) const;
    //QPolygon map(const QPolygon &a) const;
    //QRegion map(const QRegion &r) const;
    //QPainterPath map(const QPainterPath &p) const;
    //QPolygon mapToPolygon(const QRect &r) const;

    void reset();
    inline bool isIdentity() const;

    Matrix &translate(float dx, float dy);
    Matrix &scale(float sx, float sy);
    Matrix &shear(float sh, float sv);
    Matrix &rotate(float a);

    //bool isInvertible() const { return !qFuzzyIsNull(_m11*_m22 - _m12*_m21); }
    float determinant() const { return _m11*_m22 - _m12*_m21; }

    Matrix inverted(bool *invertible = 0) const;

    bool operator==(const Matrix &) const;
    bool operator!=(const Matrix &) const;

    Matrix &operator*=(const Matrix &);
    Matrix operator*(const Matrix &o) const;

    Matrix &operator=(const Matrix &);

    //operator QVariant() const;

private:
    inline Matrix(bool)
            : _m11(1.)
            , _m12(0.)
            , _m21(0.)
            , _m22(1.)
            , _dx(0.)
            , _dy(0.) {}
    inline Matrix(float am11, float am12, float am21, float am22, float adx, float ady, bool)
            : _m11(am11)
            , _m12(am12)
            , _m21(am21)
            , _m22(am22)
            , _dx(adx)
            , _dy(ady) {}
    friend class Transform;
    float _m11, _m12;
    float _m21, _m22;
    float _dx, _dy;
};
//Q_DECLARE_TYPEINFO(QMatrix, Q_MOVABLE_TYPE);

// mathematical semantics
/*Q_GUI_EXPORT_INLINE QPoint operator*(const QPoint &p, const QMatrix &m)
{ return m.map(p); }
Q_GUI_EXPORT_INLINE QPointF operator*(const QPointF &p, const QMatrix &m)
{ return m.map(p); }
Q_GUI_EXPORT_INLINE QLineF operator*(const QLineF &l, const QMatrix &m)
{ return m.map(l); }
Q_GUI_EXPORT_INLINE QLine operator*(const QLine &l, const QMatrix &m)
{ return m.map(l); }
Q_GUI_EXPORT_INLINE QPolygon operator *(const QPolygon &a, const QMatrix &m)
{ return m.map(a); }
Q_GUI_EXPORT_INLINE QPolygonF operator *(const QPolygonF &a, const QMatrix &m)
{ return m.map(a); }
Q_GUI_EXPORT_INLINE QRegion operator *(const QRegion &r, const QMatrix &m)
{ return m.map(r); }
Q_GUI_EXPORT QPainterPath operator *(const QPainterPath &p, const QMatrix &m);*/

/*inline bool Matrix::isIdentity() const
{
    return qFuzzyIsNull(_m11 - 1) && qFuzzyIsNull(_m22 - 1) && qFuzzyIsNull(_m12)
           && qFuzzyIsNull(_m21) && qFuzzyIsNull(_dx) && qFuzzyIsNull(_dy);
}*/

/*inline bool qFuzzyCompare(const QMatrix& m1, const QMatrix& m2)
{
    return qFuzzyCompare(m1.m11(), m2.m11())
        && qFuzzyCompare(m1.m12(), m2.m12())
        && qFuzzyCompare(m1.m21(), m2.m21())
        && qFuzzyCompare(m1.m22(), m2.m22())
        && qFuzzyCompare(m1.dx(), m2.dx())
        && qFuzzyCompare(m1.dy(), m2.dy());
}*/


/*****************************************************************************
 QMatrix stream functions
 *****************************************************************************/

/*#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QMatrix &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QMatrix &);
#endif

#ifndef QT_NO_DEBUG_STREAM
Q_GUI_EXPORT QDebug operator<<(QDebug, const QMatrix &);
#endif

#ifdef QT3_SUPPORT
QT_BEGIN_INCLUDE_NAMESPACE
#include <QtGui/qwmatrix.h>
QT_END_INCLUDE_NAMESPACE
#endif

QT_END_NAMESPACE

QT_END_HEADER*/

#endif  