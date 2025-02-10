//
// Created by pippin on 2025/02/02.
//

#ifndef SEAMLY2D_BLENDERVPIECE_H
#define SEAMLY2D_BLENDERVPIECE_H

#include <QVector3D>
#include <QString>
#include <QPair>

class BlenderVPiece {

public:
    BlenderVPiece(float x = .0f, float y = .0f, float z = .0f,
                  float rotationX = .0f, float rotationY = .0f, float rotationZ = .0f,
                  QPair<QString, QString> anchoringPoint = QPair<QString, QString>());

    void setPosition(float x, float y, float z);
    void setRotation(float rotationX, float rotationY, float rotationZ);
    void setAnchoringPoint(QPair<QString, QString> anchoringPoint);

    QVector3D getPosition() const;
    QVector3D getRotation() const;
    QPair<QString, QString> getAnchoringPoint() const;

private:
    QVector3D position;
    QVector3D rotation;
    QPair<QString, QString> anchoringPoint;
};


#endif //SEAMLY2D_BLENDERVPIECE_H
