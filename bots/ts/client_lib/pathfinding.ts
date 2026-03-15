import {getGame, coreGetObjFromPos} from './client_lib';
import {ObjType, Pos} from './types';

// ---------- COST FUNCTION ----------
function movementCostAt(pos: Pos): number {
    const anyObj = coreGetObjFromPos(pos);
    if (!anyObj) {
        return 1; // empty tile
    } else {
        switch (anyObj.type) {
            case ObjType.WALL:
            case ObjType.CORE:
            case ObjType.RESOURCE: // OBJ_DEPOSIT in C
                return Infinity; // impassable
            case ObjType.UNIT:
            case ObjType.MONEY: // OBJ_GEM_PILE in C
            case ObjType.BOMB:
                return 2; // passable, but costs more
            default:
                return 1; // unknown object, assume passable
        }
    }
}

function posToIndex(p: Pos, gridSize: number): number {
    return p.y * gridSize + p.x;
}

function indexToPos(idx: number, gridSize: number): Pos {
    return {
        x: idx % gridSize,
        y: Math.floor(idx / gridSize)
    };
}

function inBounds(x: number, y: number, grid: number): boolean {
    return x >= 0 && y >= 0 && x < grid && y < grid;
}

function posInBounds(p: Pos, grid: number): boolean {
    return p.x >= 0 && p.y >= 0 && p.x < grid && p.y < grid;
}

function reconstructFullPath(parent: Int32Array, startIdx: number, goalIdx: number, grid: number): Pos[] {
    const path: Pos[] = [];
    let cur = goalIdx;

    while (cur !== startIdx && parent[cur] !== -1) {
        path.push(indexToPos(cur, grid));
        cur = parent[cur];
    }

    return path.reverse();
}

/**
 * Returns the full path towards target using Dijkstra's algorithm.
 */
export function pathfindFullPathDijkstra(start: Pos, target: Pos): Pos[] {
    const game = getGame();
    const grid = game.config.gridSize;
    const total = grid * grid;

    if (grid === 0 || !posInBounds(start, grid) || !posInBounds(target, grid)) return [];
    if (start.x === target.x && start.y === target.y) return [];

    const distance = new Float64Array(total).fill(Infinity);
    const visited = new Uint8Array(total).fill(0);
    const parent = new Int32Array(total).fill(-1);

    const startIdx = posToIndex(start, grid);
    const targetIdx = posToIndex(target, grid);
    distance[startIdx] = 0;

    while (true) {
        let current = -1;
        let bestDist = Infinity;

        for (let i = 0; i < total; ++i) {
            if (visited[i] === 0 && distance[i] < bestDist) {
                bestDist = distance[i];
                current = i;
            }
        }

        if (current === -1 || bestDist === Infinity) break;
        if (current === targetIdx) break;

        visited[current] = 1;

        const curPos = indexToPos(current, grid);
        const dx = [1, -1, 0, 0];
        const dy = [0, 0, 1, -1];

        for (let dir = 0; dir < 4; ++dir) {
            const nx = curPos.x + dx[dir];
            const ny = curPos.y + dy[dir];
            if (!inBounds(nx, ny, grid)) continue;

            const neighbor: Pos = {x: nx, y: ny};
            const nIdx = posToIndex(neighbor, grid);
            if (visited[nIdx] === 1) continue;

            let stepCost = movementCostAt(neighbor);
            // In C logic: if it's the target, cost is 1 (to be able to reach it even if it's impassable like a Core or Wall?)
            // Wait, let's check pathfinding.c line 148: if (neighbor.x == target.x && neighbor.y == target.y) step_cost = 1;
            if (neighbor.x === target.x && neighbor.y === target.y) stepCost = 1;
            
            if (stepCost === Infinity) continue;

            if (distance[current] + stepCost < distance[nIdx]) {
                distance[nIdx] = distance[current] + stepCost;
                parent[nIdx] = current;
            }
        }
    }

    // Determine goal (prefer target, fallback to adjacent if unreachable)
    let goalIdx = targetIdx;

    if (distance[targetIdx] === Infinity) {
        let best = Infinity;
        let bestIdx = -1;

        const dx = [1, -1, 0, 0];
        const dy = [0, 0, 1, -1];

        for (let dir = 0; dir < 4; ++dir) {
            const nx = target.x + dx[dir];
            const ny = target.y + dy[dir];
            if (!inBounds(nx, ny, grid)) continue;

            const adj: Pos = {x: nx, y: ny};
            const adjIdx = posToIndex(adj, grid);

            if (distance[adjIdx] !== Infinity && distance[adjIdx] < best) {
                best = distance[adjIdx];
                bestIdx = adjIdx;
            }
        }

        if (bestIdx !== -1) goalIdx = bestIdx;
    }

    if (distance[goalIdx] === Infinity) return [];

    return reconstructFullPath(parent, startIdx, goalIdx, grid);
}
