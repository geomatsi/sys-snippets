project('tcp', 'c',
  version : '0.2',
  default_options : ['c_std=gnu99', 'buildtype=debug'],
)

poll_src = 's_poll.c'
fork_src = 's_fork.c'
select_src = 's_select.c'

executable('s_fork',  fork_src)
executable('s_poll',  poll_src)
executable('s_select',  select_src)

client_src = 'c_select.c'

executable('c_select', client_src)
