import * as net from 'net';
import type { GameState, Action, Obj, Pos } from './types';
import { ActionType, UnitType, ObjType, ObjState } from './types';

export class ClientLib {
    private socket: net.Socket | null = null;
    private game: GameState | null = null;
    private actions: Action[] = [];
    private debugData: any[] = [];
    private buffer: string = '';

    constructor(private teamName: string, private teamId: number, private debug: boolean = false) {}

    public async connect(host: string = '127.0.0.1', port: number = 4444): Promise<void> {
        return new Promise((resolve, reject) => {
            this.socket = new net.Socket();

            this.socket.connect(port, host, () => {
                if (this.debug) console.log(`Connected to ${host}:${port}`);
                this.sendLogin();
            });

            this.socket.on('data', (data: Buffer) => {
                this.handleData(data);
                if (this.game && resolve) {
                    resolve();
                    (resolve as any) = null;
                }
            });

            this.socket.on('error', (err) => {
                console.error('Socket error:', err);
                reject(err);
            });

            this.socket.on('close', () => {
                if (this.debug) console.log('Connection closed');
                process.exit(0);
            });
        });
    }

    private sendLogin(): void {
        const loginMsg = {
            password: "42",
            id: this.teamId,
            name: this.teamName
        };
        this.sendJson(loginMsg);
    }

    private sendJson(obj: any): void {
        if (this.socket) {
            const str = JSON.stringify(obj) + '\n';
            this.socket.write(str);
            if (this.debug) console.log('Sent:', str);
        }
    }

    private handleData(data: Buffer): void {
        this.buffer += data.toString();
        let newlineIndex;
        while ((newlineIndex = this.buffer.indexOf('\n')) !== -1) {
            const line = this.buffer.slice(0, newlineIndex);
            this.buffer = this.buffer.slice(newlineIndex + 1);
            if (line.trim()) {
                this.handleLine(line);
            }
        }
    }

    private handleLine(line: string): void {
        try {
            const parsed = JSON.parse(line);
            if (this.debug) console.log('Received:', line);

            if (!this.game) {
                // Initial config
                this.game = {
                    elapsed_ticks: 0,
                    config: parsed,
                    my_team_id: this.teamId,
                    objects: []
                };
                if (this.debug) console.log('Config received');
                // Send first packet of actions right after receiving config to start game loop
                this.sendActions();
            } else {
                // Game state update
                if (parsed.tick !== undefined) {
                    this.game.elapsed_ticks = parsed.tick;
                }
                
                if (parsed.objects && Array.isArray(parsed.objects)) {
                    for (const diff of parsed.objects) {
                        this.applyDiff(diff);
                    }
                }

                // print errors from last tick if there were any
                if (parsed.errors && Array.isArray(parsed.errors)) {
                    for (const error of parsed.errors) {
                        console.error(`\x1b[31m${error}\x1b[0m`);
                    }
                }

                // Decrement cooldowns manually as C lib does
                for (const obj of this.game.objects) {
                    if (obj.type === ObjType.UNIT) {
                         if (obj.s_unit.action_cooldown > 0) obj.s_unit.action_cooldown--;
                    } else if (obj.type === ObjType.CORE) {
                         if (obj.s_core.spawn_cooldown > 0) obj.s_core.spawn_cooldown--;
                    }
                }
                
                // After state update, we should have a tick
                if (this.tickCallback) {
                    this.tickCallback(this.game);
                }
                // ALWAYS send actions back to the server to prevent timeouts
                this.sendActions();
            }
        } catch (e) {
            console.error('Error parsing JSON:', e, 'Line:', line);
        }
    }

    private applyDiff(diff: any): void {
        if (!this.game) return;

        const id = diff.id;
        if (id === undefined) return;

        if (diff.state === 'dead') {
            this.game.objects = this.game.objects.filter(o => o.id !== id);
            return;
        }

        let obj = this.game.objects.find(o => o.id === id);
        if (!obj) {
            obj = { 
                id,
                state: ObjState.ALIVE,
                pos: { x: 0, y: 0 },
                s_core: { team_id: 0, gems: 0, spawn_cooldown: 0, balance: 0 },
                s_unit: { unit_type: 0, team_id: 0, gems: 0, action_cooldown: 0, balance: 0 },
                s_deposit_gems_pile: { gems: 0 },
                s_bomb: { countdown: 0 }
            } as Obj;
            this.game.objects.push(obj);
        }

        if (diff.type !== undefined) obj.type = diff.type;
        if (diff.x !== undefined || diff.y !== undefined) {
            if (diff.x !== undefined) obj.pos.x = diff.x;
            if (diff.y !== undefined) obj.pos.y = diff.y;
        }
        if (diff.hp !== undefined) obj.hp = diff.hp;
        
        if (diff.teamId !== undefined) {
            obj.s_core.team_id = diff.teamId;
            obj.s_unit.team_id = diff.teamId;
        }
        if (diff.gems !== undefined) {
            obj.s_core.gems = diff.gems;
            obj.s_core.balance = diff.gems;
            obj.s_unit.gems = diff.gems;
            obj.s_unit.balance = diff.gems;
            obj.s_deposit_gems_pile.gems = diff.gems;
        }
        if (diff.ActionCooldown !== undefined) obj.s_unit.action_cooldown = diff.ActionCooldown;
        if (diff.SpawnCooldown !== undefined) obj.s_core.spawn_cooldown = diff.SpawnCooldown;
        if (diff.unit_type !== undefined) obj.s_unit.unit_type = diff.unit_type;
        if (diff.countdown !== undefined) obj.s_bomb.countdown = diff.countdown;
    }

    private tickCallback: ((game: GameState) => void) | null = null;

    public startGame(callback: (game: GameState) => void): void {
        this.tickCallback = callback;
    }

    private sendActions(): void {
        const packet = {
            actions: this.actions,
            debug_data: this.debugData
        };
        this.sendJson(packet);
        this.actions = [];
        this.debugData = [];
    }

    public createUnit(unitType: UnitType): void {
        this.actions.push({
            type: ActionType.CREATE,
            unit_type: unitType
        });
    }

    public move(unit: Obj, pos: Pos): void {
        this.actions.push({
            type: ActionType.MOVE,
            unit_id: unit.id,
            x: pos.x,
            y: pos.y
        });
    }

    public attack(attacker: Obj, target: Obj): void {
        this.actions.push({
            type: ActionType.ATTACK,
            unit_id: attacker.id,
            target_id: target.id
        });
    }

    public transferGems(source: Obj, targetPos: Pos, amount: number): void {
        this.actions.push({
            type: ActionType.TRANSFER,
            source_id: source.id,
            x: targetPos.x,
            y: targetPos.y,
            amount: amount
        });
    }

    public build(builder: Obj, pos: Pos): void {
        this.actions.push({
            type: ActionType.BUILD,
            unit_id: builder.id,
            x: pos.x,
            y: pos.y
        });
    }

    public addDebugData(data: any): void {
        this.debugData.push(data);
    }

    public core_debug_addObjectInfo(obj: Obj, info: string): void {
        let entry = this.debugData.find(d => d.object_id === obj.id);
        if (!entry) {
            entry = {
                object_id: obj.id,
                object_info: "",
                object_path: []
            };
            this.debugData.push(entry);
        }
        entry.object_info += info;
    }

    public core_debug_addObjectPathStep(unit: Obj, pos: Pos): void {
        let entry = this.debugData.find(d => d.object_id === unit.id);
        if (!entry) {
            entry = {
                object_id: unit.id,
                object_info: "",
                object_path: []
            };
            this.debugData.push(entry);
        }
        entry.object_path.push({ x: pos.x, y: pos.y });
    }

    public getGame(): GameState | null {
        return this.game;
    }
}
