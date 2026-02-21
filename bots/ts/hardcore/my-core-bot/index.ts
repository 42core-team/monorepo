import { ClientLib } from '../../../core/ts/client_lib/client_lib';
import { UnitType, ObjType, GameState } from '../../../core/ts/client_lib/types';

const teamId = process.argv.length >= 3 ? Number(process.argv[2]) : 0;
if (isNaN(teamId)) {
    console.error("Invalid team id. Usage: ./bot <team_id>");
    process.exit(1);
}

const host = process.env.SERVER_IP || '127.0.0.1';
const port = process.env.SERVER_PORT ? parseInt(process.env.SERVER_PORT) : 4444;
const bot = new ClientLib("Hardcore TS Core Bot", teamId, true);

bot.startGame((game: GameState) => {
    // Create a warrior
    bot.createUnit(UnitType.WARRIOR);

    // Hardcore bot logic
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
