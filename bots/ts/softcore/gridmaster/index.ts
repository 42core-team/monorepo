import {UnitType, ObjType} from '@core/client-lib/types';
import {getGame, startGame, coreCreateUnit, coreActionMove} from '@core/client-lib/client_lib';

function onTick() {
    coreCreateUnit(UnitType.WARRIOR);

    const opponentCore = getGame().objects?.find(obj =>
        obj.type === ObjType.CORE && obj.s_core.team_id !== getGame()?.my_team_id
    );

    if (opponentCore) {
        getGame().objects.forEach(obj => {
            if (obj.type === ObjType.UNIT && obj.s_unit.team_id === getGame().my_team_id) {
                coreActionMove(obj, opponentCore.pos);
            }
        });
    }
}

startGame("Gridmaster TS Core Bot", onTick)
