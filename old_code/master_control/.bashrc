# ~/.bashrc: executed by bash(1) for non-login shells.

# Note: PS1 and umask are already set in /etc/profile. You should not
# need this unless you want different defaults for root.
# PS1='${debian_chroot:+($debian_chroot)}\h:\w\$ '
# umask 022

# You may uncomment the following lines if you want `ls' to be colorized:
# export LS_OPTIONS='--color=auto'
# eval "`dircolors`"
# alias ls='ls $LS_OPTIONS'
# alias ll='ls $LS_OPTIONS -lA'
# alias l='ls $LS_OPTIONS -l'
#
# Some more alias to avoid making mistakes:
# alias rm='rm -i'
# alias cp='cp -i'
# alias mv='mv -i'


/sbin/route del default gw 192.168.7.1
/sbin/route add default gw 192.168.7.1

if ! grep -q 8.8.8.8 /etc/resolv.conf; then
	echo nameserver 8.8.8.8 >> /etc/resolv.conf 
fi

alias ls='ls -F --color=tty --show-control-chars'
alias ll='ls -al'
alias l='ls -l'
