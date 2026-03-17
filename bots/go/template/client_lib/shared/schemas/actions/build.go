package actions

type ActionBuild struct {
	Type   string `json:"type"` // "build"‚
	UnitId uint   `json:"unit_id"`
	X      uint   `json:"x"`
	Y      uint   `json:"y"`
}

func NewActionBuild(unitId uint, x uint, y uint) ActionBuild {
	return ActionBuild{
		Type:   "build",
		UnitId: unitId,
		X:      x,
		Y:      y,
	}
}
