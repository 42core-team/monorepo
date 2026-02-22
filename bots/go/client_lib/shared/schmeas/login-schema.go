package schmeas

import "encoding/json"

type LoginRequest struct {
	Id       uint   `json:"id"`
	Password string `json:"password"`
	TeamName string `json:"name"`
}

func (l LoginRequest) Marshal() ([]byte, error) {
	return json.Marshal(l)
}

func NewLoginRequest(id uint, teamName string) LoginRequest {
	return LoginRequest{
		Id:       id,
		Password: "42",
		TeamName: teamName,
	}
}
