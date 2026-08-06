package game

import (
	"encoding/json"
	"testing"
)

func TestConfigDecodesRenamedGemFields(t *testing.T) {
	var config Config
	err := json.Unmarshal([]byte(`{
		"gridSize":20,
		"initialGems":250,
		"components":{
			"unitDefaultProperties":{
				"hp":5,
				"baseActionCooldown":3,
				"gemsPerCooldownStep":15,
				"maxGems":40,
				"damageReductionPercent":0,
				"damageCore":1,
				"damageUnit":2,
				"damageObject":3,
				"postSpawnCoreCooldown":4
			},
			"components":[]
		}
	}`), &config)
	if err != nil {
		t.Fatal(err)
	}
	properties := config.Components.UnitDefaultProperties
	if config.InitialGems != 250 || properties.GemsPerCooldownStep != 15 || properties.MaxGems != 40 {
		t.Fatalf("renamed config fields not decoded: %#v", config)
	}
}
