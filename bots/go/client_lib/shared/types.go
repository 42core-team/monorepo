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

type UnitType int

const (
	UnitWarrior UnitType = iota
	UnitMiner
	UnitCarrier
	UnitTank
)
