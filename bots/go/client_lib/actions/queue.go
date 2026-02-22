package actions

import "github.com/42core-team/go-client-lib/shared/schmeas/actions"

type ActionQueue struct {
	queue      chan actions.Action
	bufferSize int
}

func NewActionQueue(bufferSize int) *ActionQueue {
	return &ActionQueue{
		queue:      make(chan actions.Action, bufferSize),
		bufferSize: bufferSize,
	}
}

func (aq *ActionQueue) Add(action actions.Action) {
	aq.queue <- action
}

func (aq *ActionQueue) GetAll() []actions.Action {
	var allActions []actions.Action
	for {
		select {
		case action := <-aq.queue:
			if len(allActions) == aq.bufferSize {
				// TODO: send the actions when full
			}
			allActions = append(allActions, action)
		default:
			return allActions
		}
	}
}
