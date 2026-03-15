import {UnitType, ObjType, Obj} from '@core/client-lib/types';
import {
    getGame,
    startGame,
    coreCreateUnit,
    coreActionPathfind,
    coreGetObjFilterNearest,
    coreGetObjsFilter
} from '@core/client-lib/client_lib';

function isCoreOpponent(obj: Obj): boolean {
    return (obj.type === ObjType.CORE && obj.s_core.team_id !== getGame().my_team_id);
}

function isUnitOwn(obj: Obj): boolean {
    return (obj.type === ObjType.UNIT && obj.s_unit.team_id === getGame().my_team_id);
}

function onTick() {
    coreCreateUnit(UnitType.WARRIOR);

    const opponentCore = coreGetObjFilterNearest({x: 0, y: 0}, isCoreOpponent);

    if (opponentCore) {
        const units = coreGetObjsFilter(isUnitOwn);
        units.forEach(unit => {
            coreActionPathfind(unit, opponentCore.pos);
        });
    }
}

startGame("Gridmaster", onTick)
