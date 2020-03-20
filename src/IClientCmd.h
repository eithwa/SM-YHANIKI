#pragma once

namespace Yhaniki {

    class IClientCmd {
    public:
        enum class Type {
            Nop,
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
            StartFileTransfer
        };

        virtual ~IClientCmd() = default;
        virtual Type GetType() const = 0;
    };
}
