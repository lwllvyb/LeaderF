" ============================================================================
" File:        Buffer.vim
" Description:
" Author:      Yggdroot <archofortune@gmail.com>
" Website:     https://github.com/Yggdroot
" Note:
" License:     Apache License, Version 2.0
" ============================================================================

if leaderf#versionCheck() == 0  " this check is necessary
    finish
endif

exec g:Lf_py "from leaderf.bufExpl import *"

function! leaderf#Buffer#Maps()
    nmapclear <buffer>
    nnoremap <buffer> <silent> <CR>          :exec g:Lf_py "bufExplManager.accept()"<CR>
    nnoremap <buffer> <silent> o             :exec g:Lf_py "bufExplManager.accept()"<CR>
    nnoremap <buffer> <silent> <2-LeftMouse> :exec g:Lf_py "bufExplManager.accept()"<CR>
    nnoremap <buffer> <silent> x             :exec g:Lf_py "bufExplManager.accept('h')"<CR>
    nnoremap <buffer> <silent> v             :exec g:Lf_py "bufExplManager.accept('v')"<CR>
    nnoremap <buffer> <silent> t             :exec g:Lf_py "bufExplManager.accept('t')"<CR>
    nnoremap <buffer> <silent> q             :exec g:Lf_py "bufExplManager.quit()"<CR>
    " nnoremap <buffer> <silent> <Esc>         :exec g:Lf_py "bufExplManager.quit()"<CR>
    nnoremap <buffer> <silent> i             :exec g:Lf_py "bufExplManager.input()"<CR>
    nnoremap <buffer> <silent> <Tab>         :exec g:Lf_py "bufExplManager.input()"<CR>
    nnoremap <buffer> <silent> <F1>          :exec g:Lf_py "bufExplManager.toggleHelp()"<CR>
    nnoremap <buffer> <silent> d             :exec g:Lf_py "bufExplManager.deleteBuffer(1)"<CR>
    nnoremap <buffer> <silent> D             :exec g:Lf_py "bufExplManager.deleteBuffer()"<CR>
    if has_key(g:Lf_NormalMap, "Buffer")
        for i in g:Lf_NormalMap["Buffer"]
            exec 'nnoremap <buffer> <silent> '.i[0].' '.i[1]
        endfor
    endif
endfunction

function! leaderf#Buffer#NormalModeFilter(winid, key) abort
    let key = get(g:Lf_KeyDict, get(g:Lf_KeyMap, a:key, a:key), a:key)

    if key !=# "g"
        call win_execute(a:winid, "let g:Lf_Buffer_is_g_pressed = 0")
    endif

    if key ==# "j" || key ==? "<Down>"
        call win_execute(a:winid, "norm! j")
        exec g:Lf_py "bufExplManager._cli._buildPopupPrompt()"
        redraw
        exec g:Lf_py "bufExplManager._getInstance().refreshPopupStatusline()"
    elseif key ==# "k" || key ==? "<Up>"
        call win_execute(a:winid, "norm! k")
        exec g:Lf_py "bufExplManager._cli._buildPopupPrompt()"
        redraw
        exec g:Lf_py "bufExplManager._getInstance().refreshPopupStatusline()"
    elseif key ==? "<PageUp>" || key ==? "<C-B>"
        call win_execute(a:winid, "norm! \<PageUp>")
        exec g:Lf_py "bufExplManager._cli._buildPopupPrompt()"
        exec g:Lf_py "bufExplManager._getInstance().refreshPopupStatusline()"
    elseif key ==? "<PageDown>" || key ==? "<C-F>"
        call win_execute(a:winid, "norm! \<PageDown>")
        exec g:Lf_py "bufExplManager._cli._buildPopupPrompt()"
        exec g:Lf_py "bufExplManager._getInstance().refreshPopupStatusline()"
    elseif key ==# "g"
        if get(g:, "Lf_Buffer_is_g_pressed", 0) == 0
            let g:Lf_Buffer_is_g_pressed = 1
        else
            let g:Lf_Buffer_is_g_pressed = 0
            call win_execute(a:winid, "norm! gg")
            exec g:Lf_py "bufExplManager._cli._buildPopupPrompt()"
            redraw
        endif
    elseif key ==# "G"
        call win_execute(a:winid, "norm! G")
        exec g:Lf_py "bufExplManager._cli._buildPopupPrompt()"
        redraw
    elseif key ==? "<C-U>"
        call win_execute(a:winid, "norm! \<C-U>")
        exec g:Lf_py "bufExplManager._cli._buildPopupPrompt()"
        redraw
    elseif key ==? "<C-D>"
        call win_execute(a:winid, "norm! \<C-D>")
        exec g:Lf_py "bufExplManager._cli._buildPopupPrompt()"
        redraw
    elseif key ==? "<LeftMouse>"
        if has('patch-8.1.2266')
            call win_execute(a:winid, "exec v:mouse_lnum")
            call win_execute(a:winid, "exec 'norm!'.v:mouse_col.'|'")
        exec g:Lf_py "bufExplManager._cli._buildPopupPrompt()"
            redraw
        endif
    elseif key ==? "<ScrollWheelUp>"
        call win_execute(a:winid, "norm! 3k")
        exec g:Lf_py "bufExplManager._cli._buildPopupPrompt()"
        redraw
        exec g:Lf_py "bufExplManager._getInstance().refreshPopupStatusline()"
    elseif key ==? "<ScrollWheelDown>"
        call win_execute(a:winid, "norm! 3j")
        exec g:Lf_py "bufExplManager._cli._buildPopupPrompt()"
        redraw
        exec g:Lf_py "bufExplManager._getInstance().refreshPopupStatusline()"
    elseif key ==# "q" || key ==? "<ESC>"
        exec g:Lf_py "bufExplManager.quit()"
    elseif key ==# "i" || key ==? "<Tab>"
        call leaderf#ResetPopupOptions(a:winid, 'filter', 'leaderf#PopupFilter')
        exec g:Lf_py "bufExplManager.input()"
    elseif key ==# "o" || key ==? "<CR>" || key ==? "<2-LeftMouse>"
        exec g:Lf_py "bufExplManager.accept()"
    elseif key ==# "x"
        exec g:Lf_py "bufExplManager.accept('h')"
    elseif key ==# "v"
        exec g:Lf_py "bufExplManager.accept('v')"
    elseif key ==# "t"
        exec g:Lf_py "bufExplManager.accept('t')"
    elseif key ==# "p"
        exec g:Lf_py "bufExplManager._previewResult(True)"
    elseif key ==? "<F1>"
        exec g:Lf_py "bufExplManager.toggleHelp()"
    elseif key ==# "d"
        exec g:Lf_py "bufExplManager.deleteBuffer(1)"
    elseif key ==# "D"
        exec g:Lf_py "bufExplManager.deleteBuffer()"
    endif

    return 1
endfunction
