//go:build exclude

// License: GPLv3 Copyright: 2022, anders Goyal, <anders at backbiter-no.net>

// Code generated by gen-go-code.py; DO NOT EDIT.

package at

import (
	"fmt"
	"strings"
	"time"

	"smelly/tools/cli"
	"smelly/tools/utils"
)

var _ = fmt.Print
var _ = strings.Join

type options_CMD_NAME_type struct {
	OPTIONS_DECLARATION_CODE
}

var options_CMD_NAME options_CMD_NAME_type

type CMD_NAME_json_type struct {
	JSON_DECLARATION_CODE
}

func create_payload_CMD_NAME(io_data *rc_io_data, cmd *cli.Command, args []string) (err error) {
	payload := CMD_NAME_json_type{}
	JSON_INIT_CODE
	io_data.rc.Payload = payload
	return
}

func create_rc_CMD_NAME(args []string) (*utils.RemoteControlCmd, error) {
	rc := utils.RemoteControlCmd{
		Cmd:        "CLI_NAME",
		Version:    ProtocolVersion,
		NoResponse: NO_RESPONSE_BASE,
		Stream:     STREAM_WANTED,
	}
	if rc.Stream {
		stream_id, err := utils.HumanRandomId(128)
		if err != nil {
			return nil, err
		}
		rc.StreamId = stream_id
	}
	if IS_ASYNC {
		async_id, err := utils.HumanRandomId(128)
		if err != nil {
			return nil, err
		}
		rc.Async = async_id
	}
	return &rc, nil
}

func run_CMD_NAME(cmd *cli.Command, args []string) (return_code int, err error) {
	err = cmd.GetOptionValues(&options_CMD_NAME)
	if err != nil {
		return
	}

	rc, err := create_rc_CMD_NAME(args)
	if err != nil {
		return
	}
	nrv, err := cli.GetOptionValue[bool](cmd, "NoResponse")
	if err == nil {
		rc.NoResponse = nrv
	}
	var timeout float64 = WAIT_TIMEOUT
	rt, err := cli.GetOptionValue[float64](cmd, "ResponseTimeout")
	if err == nil {
		timeout = rt
	}
	io_data := rc_io_data{
		cmd:                    cmd,
		rc:                     rc,
		timeout:                time.Duration(timeout * float64(time.Second)),
		string_response_is_err: STRING_RESPONSE_IS_ERROR,
	}
	err = create_payload_CMD_NAME(&io_data, cmd, args)
	if err != nil {
		return
	}

	err = send_rc_command(&io_data)
	return
}

func setup_CMD_NAME(parent *cli.Command) *cli.Command {
	ans := parent.AddSubCommand(&cli.Command{
		Name:             "CLI_NAME",
		Usage:            "ARGSPEC",
		ShortDescription: "SHORT_DESC",
		HelpText:         "LONG_DESC",
		Run:              run_CMD_NAME,
	})
	ADD_FLAGS_CODE
	return ans
}

func init() {
	register_at_cmd(setup_CMD_NAME)
}
