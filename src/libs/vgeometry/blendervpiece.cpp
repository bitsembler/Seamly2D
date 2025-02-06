//
// Created by pippin on 2025/02/02.
//

#include "blendervpiece.h"

#include <QVector3D>
#include <QString>
#include <QPair>

BlenderVPiece::BlenderVPiece(const float x, const float y, const float z,
                               const float rotationX, const float rotationY, const float rotationZ,
                               const QPair<QString, QString> anchoringPoint)
        :position(x, y, z), rotation(rotationX, rotationY, rotationZ), anchoringPoint(anchoringPoint)
{
}

void BlenderVPiece::setPosition(float x, float y, float z)
{
    position.setX(x);
    position.setY(y);
    position.setZ(z);
}

void BlenderVPiece::setRotation(float rotationX, float rotationY, float rotationZ)
{
    rotation.setX(rotationX);
    rotation.setY(rotationY);
    rotation.setZ(rotationZ);
}

void BlenderVPiece::setAnchoringPoint(QPair<QString, QString> newAnchoringPoint) {
    anchoringPoint = newAnchoringPoint;
}

QVector3D BlenderVPiece::getPosition() const
{
    return position;
}

QVector3D BlenderVPiece::getRotation() const
{
    return rotation;
}

QPair<QString, QString> BlenderVPiece::getAnchoringPoint() const {
    return anchoringPoint;
}