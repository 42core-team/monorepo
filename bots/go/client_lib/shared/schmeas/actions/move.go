package actions

type ActionMove struct {
	Type   string `json:"type"` // "move"
	UnitId uint   `json:"unit_id"`
	X      uint   `json:"x"`
	Y      uint   `json:"y"`
}

func NewActionMove(unitId uint, x uint, y uint) ActionMove {
	return ActionMove{
		Type:   "move",
		UnitId: unitId,
		X:      x,
		Y:      y,
	}
}
