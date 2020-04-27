#include "main.h"
#include "cmdprocs.h"
#include "chatwindow.h"
#include "game/game.h"
#include "scoreboard.h"

extern CChatWindow *pChatWindow;
extern CGame *pGame;
extern CScoreBoard *pScoreBoard;

void cmdQuitGame(const char *params)
{
    // I guess I should use QuitApp instead of this
    std::terminate();
}

void cmdFrameLimit(const char *params)
{
    if(strlen(params)) {
        int framelimit = atoi(params);
        if(framelimit >= 20 && framelimit <= 90){
            *((int*)g_libGTASA+0x95B074) = framelimit;
            pChatWindow->AddInfoMessage("-> FrameLimiter: %u", framelimit);
        }
        else
        {
            pChatWindow->AddInfoMessage("-> FrameLimiter: valid amounts are 20-90");
        }
    }
}

void cmdGetState(const char *params)
{
    pChatWindow->AddDebugMessage("-> Pause State: %d, Game State: %d", pGame->IsGamePaused(), pGame->IsPlayingGame());
}

void cmdScoreBoard(const char *params)
{
    pScoreBoard->Toggle();
}

void SetupCommands()
{
    pChatWindow->AddCmdProc("q", cmdQuitGame);
    pChatWindow->AddCmdProc("quit", cmdQuitGame);
    pChatWindow->AddCmdProc("fpslimit", cmdFrameLimit);
    pChatWindow->AddCmdProc("all", cmdScoreBoard);

    #ifdef RELEASE_BETA
    //pChatWindow->AddCmdProc("getstate", cmdGetState);
    #endif
}