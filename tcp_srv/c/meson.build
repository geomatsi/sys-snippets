project('tcp', 'c', version : '0.2')

poll_src = 'srv_poll.c'
fork_src = 'srv_fork.c'
select_src = 'srv_select.c'

executable('srv_fork',  fork_src)
executable('srv_poll',  poll_src)
executable('srv_select',  select_src)

client_src = 'client.c'

executable('client', client_src)
