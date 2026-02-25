package actions

type ActionTransferGems struct {
	Type     string `json:"type"` // "transfer_gems"‚
	SourceId uint   `json:"source_id"`
	X        uint   `json:"x"`
	Y        uint   `json:"y"`
	Amount   uint   `json:"amount"` // min 1
}

func NewActionTransferGems(sourceId uint, x uint, y uint, amount uint) ActionTransferGems {
	return ActionTransferGems{
		Type:     "transfer_gems",
		SourceId: sourceId,
		X:        x,
		Y:        y,
		Amount:   amount,
	}
}
