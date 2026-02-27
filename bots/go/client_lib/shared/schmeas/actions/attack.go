package actions

type ActionAttack struct {
	Type     string `json:"type"` // "attack"‚
	UnitId   uint   `json:"unit_id"`
	TargetId uint   `json:"target_id"`
}

func NewActionAttack(unitId uint, targetId uint) ActionAttack {
	return ActionAttack{
		Type:     "attack",
		UnitId:   unitId,
		TargetId: targetId,
	}
}
