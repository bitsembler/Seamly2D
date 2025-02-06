//
// Created by juanp on 2025/02/05.
//

#include "blendermanager.h"
#include "blendervpiece.h"

#include <QSharedPointer>

BlenderManager& BlenderManager::instance() {
    static BlenderManager instance;
    return instance;
}

QSharedPointer<BlenderVPiece> BlenderManager::getBlenderPieceById(quint32 id) {
    if (!blenderPieces.contains(id))
    {
        blenderPieces.insert(id, QSharedPointer<BlenderVPiece>::create());
    }

    return blenderPieces.value(id);
}