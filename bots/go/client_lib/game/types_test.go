package game

import (
	"errors"
	"testing"
)

func TestTypedDataGetters(t *testing.T) {
	getters := []struct {
		name     string
		kind     ObjectType
		payload  ObjectData
		typedNil ObjectData
		get      func(*Object) (bool, error)
	}{
		{"unit", ObjectUnit, &UnitData{}, (*UnitData)(nil), func(o *Object) (bool, error) { d, err := o.GetUnitData(); return d != nil, err }},
		{"core", ObjectCore, &CoreData{}, (*CoreData)(nil), func(o *Object) (bool, error) { d, err := o.GetCoreData(); return d != nil, err }},
		{"deposit", ObjectDeposit, &DepositData{}, (*DepositData)(nil), func(o *Object) (bool, error) { d, err := o.GetDepositData(); return d != nil, err }},
		{"gem pile", ObjectGemPile, &DepositData{}, (*DepositData)(nil), func(o *Object) (bool, error) { d, err := o.GetDepositData(); return d != nil, err }},
	}
	for _, getter := range getters {
		t.Run(getter.name, func(t *testing.T) {
			cases := []struct {
				name string
				obj  *Object
				want error
			}{
				{"valid", &Object{ID: 7, Type: getter.kind, ObjectData: getter.payload}, nil},
				{"nil receiver", nil, ErrNilObject},
				{"wrong type with matching payload", &Object{ID: 7, Type: ObjectWall, ObjectData: getter.payload}, ErrWrongObjectType},
				{"missing payload", &Object{ID: 7, Type: getter.kind}, ErrInvalidObjectData},
				{"mismatched payload", &Object{ID: 7, Type: getter.kind, ObjectData: &UnitData{}}, ErrInvalidObjectData},
				{"typed nil", &Object{ID: 7, Type: getter.kind, ObjectData: getter.typedNil}, ErrInvalidObjectData},
			}
			// A unit needs a different payload for the mismatch case.
			if getter.kind == ObjectUnit {
				cases[4].obj.ObjectData = &CoreData{}
			}
			for _, tc := range cases {
				t.Run(tc.name, func(t *testing.T) {
					present, err := getter.get(tc.obj)
					if !errors.Is(err, tc.want) || present != (tc.want == nil) {
						t.Fatalf("present=%v, err=%v; want error %v", present, err, tc.want)
					}
				})
			}
		})
	}
}
