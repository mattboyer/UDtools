# Copyright (C) 2012 Matt Boyer.
# All rights reserved.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the project nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.

# THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

function mounted_filesystems {
	FS=$(udenum -f -l -m)
	(( $? == 0 )) || return

	stem=${2}
	index=0;
	for dev in ${FS}; do
		[[ $dev == $stem* ]] || continue
		COMPREPLY[${index}]=${dev}
		index=$((1+$index))
	done
}

function unmounted_filesystems {
	FS=$(udenum -f -l +m)
	(( $? == 0 )) || return

	stem=${2}
	index=0;
	for dev in ${FS}; do
		[[ $dev == $stem* ]] || continue
		COMPREPLY[${index}]=${dev}
		index=$((1+$index))
	done
}

function partitions_without_fs {
	FS=$(udenum -p -l +k)
	(( $? == 0 )) || return

	stem=${2}
	index=0;
	for dev in ${FS}; do
		[[ $dev == $stem* ]] || continue
		COMPREPLY[${index}]=${dev}
		index=$((1+$index))
	done
}

function partitions_with_fs {
	FS=$(udenum -p -l -k)
	(( $? == 0 )) || return

	stem=${2}
	index=0;
	for dev in ${FS}; do
		[[ $dev == $stem* ]] || continue
		COMPREPLY[${index}]=${dev}
		index=$((1+$index))
	done
}

complete -F mounted_filesystems umount
complete -F unmounted_filesystems mount
complete -F unmounted_filesystems udisksctl
complete -F partitions_without_fs mkfs
complete -F partitions_with_fs fsck
complete -F partitions_with_fs fsadm
