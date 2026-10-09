// Enables an already created hook.
    // Parameters:
    //   pTarget [in] A pointer to the target function.
    //                If this parameter is VX_ALL_HOOKS, all created hooks are
    //                enabled in one go.
    VX_STATUS WINAPI MH_EnableHook(LPVOID pTarget);

    // Disables an already created hook.
    // Parameters:
    //   pTarget [in] A pointer to the target function.
    //                If this parameter is VX_ALL_HOOKS, all created hooks are
    //                disabled in gfo.
    VX_STATUS WINAPI VX_DisableHook(LPVOID pTarget);
