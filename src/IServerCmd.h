#pragma once

namespace Yhaniki::ServerCmd {

    class IServerCmd {
    public:
        /*enum class Type {
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
        */

        virtual ~IServerCmd() = default;

        //virtual Type GetType() const = 0;
    };
}
