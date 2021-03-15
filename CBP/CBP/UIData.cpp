#include "pch.h"

namespace CBP
{
    const char* UIBase::GetHelpText(MiscHelpText a_code)
    {
        switch (a_code)
        {
        case MiscHelpText::timeTick: return "Target update rate. Setting this below 60 is not recommended.";
        case MiscHelpText::maxSubSteps: return "";
        case MiscHelpText::timeScale: return "Simulation rate, speeds up or slows down time";
        case MiscHelpText::colMaxPenetrationDepth: return "Maximum penetration depth during collisions";
        case MiscHelpText::showAllActors: return "Checked: Show all known actors\nUnchecked: Only show actors currently simulated";
        case MiscHelpText::clampValues: return "Clamp slider values to the default range.";
        case MiscHelpText::syncMinMax: return "Move weighted sliders together.";
        case MiscHelpText::rescanActors: return "Scan for nearby actors and update list.";
        case MiscHelpText::resetConfOnActor: return "Clear configuration for currently selected actor.";
        case MiscHelpText::resetConfOnRace: return "Clear configuration for currently selected race.";
        case MiscHelpText::showEDIDs: return "Show editor ID's instead of names.";
        case MiscHelpText::playableOnly: return "Show playable races only.";
        case MiscHelpText::colGroupEditor: return "Nodes assigned to the same group will not collide with eachother. This applies only to nodes on the same actor.";
        case MiscHelpText::importData: return "Import and apply actor, race and global settings from the selected file.";
        case MiscHelpText::exportDialog: return "Export actor, race and global settings.";
        case MiscHelpText::simRate: return "If this value isn't equal to framerate the simulation speed is affected. Increase max. sub steps or adjust timeTick to get proper results.";
        case MiscHelpText::armorOverrides: return "";
        case MiscHelpText::offsetMin: return "Collider body offset (X, Y, Z, weight 0).";
        case MiscHelpText::offsetMax: return "Collider body offset (X, Y, Z, weight 100).";
        case MiscHelpText::applyForce: return "Apply force to node along the X, Y and Z axes, respectively.";
        case MiscHelpText::showNodes: return "";
        case MiscHelpText::dataFilterPhys: return "Filter by configuration group name. Press enter to apply.";
        case MiscHelpText::dataFilterNode: return "Filter by node name. Press enter to apply.";
        case MiscHelpText::frameTimer: return "Skyrim's frame timer, affected by time modifier.";
        case MiscHelpText::timePerFrame: return "Amount of time the physics simulation consumes per frame (in microseconds).";
        case MiscHelpText::rotation: return "Collider rotation in degrees around the Z, Y and Y axes respectively.";
        case MiscHelpText::controllerStats: return "Physics controller prints information to the log. Use this only for debugging.";
        default: return "??";
        }
    }

    const keyDesc_t UIBase::m_comboKeyDesc({
        {0, "None"},
        {DIK_LSHIFT, "Left shift"},
        {DIK_RSHIFT, "Right shift"},
        {DIK_LCONTROL, "Left control"},
        {DIK_RCONTROL, "Right control"},
        {DIK_LALT, "Left alt"},
        {DIK_RALT, "Right alt"}
        });

    const keyDesc_t UIBase::m_keyDesc({
        {DIK_INSERT,"Insert"},
        {DIK_DELETE,"Delete"},
        {DIK_HOME,"Home"},
        {DIK_END,"End"},
        {DIK_PGUP,"Page up"},
        {DIK_PGDN,"Page down"},
        {DIK_BACKSPACE, "Backspace"},
        {DIK_RETURN,"Return"},
        {DIK_PAUSE, "Pause"},
        {DIK_CAPSLOCK, "Caps lock"},
        {DIK_LEFT, "Left"},
        {DIK_RIGHT, "Right"},
        {DIK_UP, "Up"},
        {DIK_DOWN, "Down"},
        {DIK_TAB, "Tab"},
        {DIK_F1, "F1"},
        {DIK_F2, "F2"},
        {DIK_F3, "F3"},
        {DIK_F4, "F4"},
        {DIK_F5, "F5"},
        {DIK_F6, "F6"},
        {DIK_F7, "F7"},
        {DIK_F8, "F8"},
        {DIK_F9, "F9"},
        {DIK_F10, "F10"},
        {DIK_F11, "F11"},
        {DIK_F12, "F12"},
        {DIK_F13, "F13"},
        {DIK_F14, "F14"},
        {DIK_F15, "F15"},
        {DIK_NUMPAD0, "Num 0"},
        {DIK_NUMPAD1, "Num 1"},
        {DIK_NUMPAD2, "Num 2"},
        {DIK_NUMPAD3, "Num 3"},
        {DIK_NUMPAD4, "Num 4"},
        {DIK_NUMPAD5, "Num 5"},
        {DIK_NUMPAD6, "Num 6"},
        {DIK_NUMPAD7, "Num 7"},
        {DIK_NUMPAD8, "Num 8"},
        {DIK_NUMPAD9, "Num 9"},
        {DIK_NUMPADSLASH, "Num /"},
        {DIK_NUMPADSTAR, "Num *"},
        {DIK_NUMPADMINUS, "Num -"},
        {DIK_NUMPADPLUS, "Num +"},
        {DIK_NUMPADENTER, "Num Enter"},
        {DIK_NUMPADCOMMA, "Num ,"},
        {DIK_PERIOD, "."},
        {DIK_COMMA, ","},
        {DIK_MINUS, "-"},
        {DIK_BACKSLASH, "\\"},
        {DIK_COLON, ":"},
        {DIK_SEMICOLON, ";"},
        {DIK_SLASH, "/"},
        {DIK_0,"0"},
        {DIK_1,"1"},
        {DIK_2,"2"},
        {DIK_3,"3"},
        {DIK_4,"4"},
        {DIK_5,"5"},
        {DIK_6,"6"},
        {DIK_7,"7"},
        {DIK_8,"8"},
        {DIK_9,"9"},
        {DIK_A,"A"},
        {DIK_B,"B"},
        {DIK_C,"C"},
        {DIK_D,"D"},
        {DIK_E,"E"},
        {DIK_F,"F"},
        {DIK_G,"G"},
        {DIK_H,"H"},
        {DIK_I,"I"},
        {DIK_J,"J"},
        {DIK_K,"K"},
        {DIK_L,"L"},
        {DIK_M,"M"},
        {DIK_N,"N"},
        {DIK_O,"O"},
        {DIK_P,"P"},
        {DIK_Q,"Q"},
        {DIK_R,"R"},
        {DIK_S,"S"},
        {DIK_T,"T"},
        {DIK_U,"U"},
        {DIK_V,"V"},
        {DIK_W,"W"},
        {DIK_X,"X"},
        {DIK_Y,"Y"},
        {DIK_Z,"Z"}
        });


}