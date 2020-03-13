#pragma once

namespace Reimplementation {

    class ClientCmd {
    public:
        enum class Type {
            Ping, PingR,
            Hello,
            GameStartRequest,
            GameOver,
            Chat,
            SelectSong,
            LeaveLobby,
            AskSong,
            UpdateStats,
            UpdateStyle,
            UpdateOptions,
            UpdatePercentage,
            UpdateGraph,
            UpdateHasSong,
            StartFileTransfer,
        };
    };
}