package actions

import "encoding/json"

type Action interface {
	GetType() string
	Marshal() ([]byte, error)
}

func (a ActionAttack) GetType() string {
	return a.Type
}

func (a ActionBuild) GetType() string {
	return a.Type
}

func (a ActionMove) GetType() string {
	return a.Type
}

func (a ActionCreate) GetType() string {
	return a.Type
}

func (a ActionTransferGems) GetType() string {
	return a.Type
}

func (a ActionAttack) Marshal() ([]byte, error) {
	return json.Marshal(a)
}

func (a ActionBuild) Marshal() ([]byte, error) {
	return json.Marshal(a)
}

func (a ActionMove) Marshal() ([]byte, error) {
	return json.Marshal(a)
}

func (a ActionCreate) Marshal() ([]byte, error) {
	return json.Marshal(a)
}

func (a ActionTransferGems) Marshal() ([]byte, error) {
	return json.Marshal(a)
}
