//
// Created by juanp on 2025/02/05.
//

#ifndef SEAMLY2D_BLENDERMANAGER_H
#define SEAMLY2D_BLENDERMANAGER_H

#include "blendervpiece.h"

#include <QMap>
#include <QSharedPointer>

class BlenderManager {
public:
    static BlenderManager &instance();

    QSharedPointer<BlenderVPiece> getBlenderPieceById(quint32 id);

private:
    QMap<quint32, QSharedPointer<BlenderVPiece>> blenderPieces;

    BlenderManager() {}

    ~BlenderManager() {}

    Q_DISABLE_COPY(BlenderManager)
};


#endif //SEAMLY2D_BLENDERMANAGER_H
