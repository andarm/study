" All system-wide defaults are set in $VIMRUNTIME/debian.vim (usually just
" /usr/share/vim/vimcurrent/debian.vim) and sourced by the call to :runtime
" you can find below.  If you wish to change any of those settings, you should
" do it in this file (/etc/vim/vimrc), since debian.vim will be overwritten
" everytime an upgrade of the vim packages is performed.  It is recommended to
" make changes after sourcing debian.vim since it alters the value of the
" 'compatible' option.

" This line should not be removed as it ensures that various options are
" properly set to work with the Vim-related packages available in Debian.
runtime! debian.vim

" Uncomment the next line to make Vim more Vi-compatible
" NOTE: debian.vim sets 'nocompatible'.  Setting 'compatible' changes numerous
" options, so any other options should be set AFTER setting 'compatible'.
"set compatible

" Vim5 and later versions support syntax highlighting. Uncommenting the
" following enables syntax highlighting by default.
if has("syntax")
  syntax on
endif

" If using a dark background within the editing area and syntax highlighting
" turn on this option as well
"set background=dark

" Uncomment the following to have Vim jump to the last position when
" reopening a file
"if has("autocmd")
"  au BufReadPost * if line("'\"") > 1 && line("'\"") <= line("$") | exe "normal! g'\"" | endif
"endif

" Uncomment the following to have Vim load indentation rules and plugins
" according to the detected filetype.
"if has("autocmd")
"  filetype plugin indent on
"endif

" The following are commented out as they cause vim to behave a lot
" differently from regular Vi. They are highly recommended though.
"set showcmd		" Show (partial) command in status line.
"set showmatch		" Show matching brackets.
"set ignorecase		" Do case insensitive matching
"set smartcase		" Do smart case matching
"set incsearch		" Incremental search
"set autowrite		" Automatically save before commands like :next and :make
"set hidden             " Hide buffers when they are abandoned
"set mouse=a		" Enable mouse usage (all modes)

" Source a global configuration file if available
if filereadable("/etc/vim/vimrc.local")
  source /etc/vim/vimrc.local
endif

set fileencodings=utf-8,gb2312,gbk,gb18030
set termencoding=utf-8
set encoding=prc
"set nu
syntax on  
"autocmd InsertLeave
filetype plugin on
"let Tlist_Auto_Open=1
autocmd InsertEnter * se cul
set showcmd
set autoindent
set completeopt=longest,menu
"map llo :TlistOpen<CR>
"map llc :TlistClose<CR>
colo desert
"map <F5> I# <ESC>
"map =  ^i//<esc>
"for omni
"set nocp  
"#########################
"set cscopequickfix=s-,c-,d-,i-,t-,e-
"nmap s :cs find s =expand("")
"nmap g :cs find g =expand("")
"nmap c :cs find c =expand("")
"nmap t :cs find t =expand("")
"nmap e :cs find e =expand("")
"nmap f :cs find f =expand("")
"nmap i :cs find i ^=expand("")$
"nmap d :cs find d =expand("")

set autoindent "开启自动缩进
set shiftwidth=4 "自动缩进4个空格
set softtabstop=4 "suo


set tags=tags;
set autochdir
set nocompatible              " required
filetype off                  " required

set rtp+=~/.vim/bundle/Vundle.vim
call vundle#begin()
Plugin 'gmarik/Vundle.vim'
Plugin 'linuxsty.vim'
Plugin 'ctags.vim'
Plugin 'vim-snipmate'
Plugin 'inccomplete'
Plugin 'Valloric/YouCompleteMe'

call vundle#end()            " required
filetype plugin indent on    " required
set nu
set hlsearch  
set mouse=a
imap <F5> :<ESC>:wa<CR>:call Do_OneFileCompile()<CR>

function Do_OneFileCompile()
    if expand("%:p:h") != getcwd()
	echohl WarningMsg | echo "Fail to compile, no such file in current directory!"
	| echohl none
	exit -1
    endif
    let sourcefile_name = expand("%:t")
    if (sourcefile_name == "" || (&filetype != "c" && &filetype != "cpp"))
	echohl WarningMsg | echo "It's not a \"c\" or \"cpp\" file!"
	| echohl none
	exit -2
    endif
    let sourcefile_name = expand("%:r")

    if &filetype == "c"
	execute "!gcc\ -g\ -o\ " . sourcefile_name . "\ %"
    elseif &filetype == "cpp"
	execute "!g++\ -g\ -o\ " . sourcefile_name . "\ %"
    endif
    let exec_filename = expand("%:p:h") . '/' . expand("%:r")
    echo exec_filename
    execute "!chmod +x " . exec_filename
    execute "!gnome-terminal -e ' " . exec_filename . "'"
endfunction 
if has("cscope")
set csprg=/usr/bin/cscope
set csto=0
set cst
set csverb
set cspc=3
endif


"set tags=tags 
