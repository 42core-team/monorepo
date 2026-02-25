package shared

type ObjectType int

const (
	ObjectCore ObjectType = iota
	ObjectUnit
	ObjectDeposit
	ObjectWall
	ObjectGemPile
	ObjectBomb
)

func (o ObjectType) String() string {
	switch o {
	case ObjectCore:
		return "Core"
	case ObjectUnit:
		return "Unit"
	case ObjectDeposit:
		return "Deposit"
	case ObjectWall:
		return "Wall"
	case ObjectGemPile:
		return "GemPile"
	case ObjectBomb:
		return "Bomb"
	default:
		return "Unknown"
	}
}

type UnitType int

const (
	UnitWarrior UnitType = iota
	UnitMiner
	UnitCarrier
	UnitTank
)

func (u UnitType) String() string {
	switch u {
	case UnitWarrior:
		return "Warrior"
	case UnitMiner:
		return "Miner"
	case UnitCarrier:
		return "Carrier"
	case UnitTank:
		return "Tank"
	default:
		return "Unknown"
	}
}
