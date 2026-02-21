import { ClientLib } from '../../../core/ts/client_lib/client_lib';
import { UnitType, ObjType, GameState } from '../../../core/ts/client_lib/types';

const bot = new ClientLib("TS Core Bot", 1);

bot.startGame((game: GameState) => {
    // Create a warrior
    bot.createUnit(UnitType.WARRIOR);

    // Simple bot logic
    const opponentCore = game.objects.find(obj => 
        obj.type === ObjType.CORE && obj.s_core.team_id !== game.my_team_id
    );

    if (opponentCore) {
        game.objects.forEach(obj => {
            if (obj.type === ObjType.UNIT && obj.s_unit.team_id === game.my_team_id) {
                bot.move(obj, opponentCore.pos);
            }
        });
    }
});

bot.connect();
