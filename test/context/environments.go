// Copyright (c) 2022 Proton AG
//
// This file is part of Proton Mail Bridge.Bridge.
//
// Proton Mail Bridge is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Proton Mail Bridge is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Proton Mail Bridge. If not, see <https://www.gnu.org/licenses/>.

package context

import (
	"os"
	"time"
)

const (
	EnvName = "TEST_ENV"
	EnvFake = "fake"
	EnvLive = "live"
)

func (ctx *TestContext) EventLoopTimeout() time.Duration {
	switch os.Getenv(EnvName) {
	case EnvFake:
		return 5 * time.Second
	case EnvLive:
		return 60 * time.Second
	default:
		panic("unknown env")
	}
}
