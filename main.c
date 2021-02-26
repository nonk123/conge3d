#include "conge/conge.h"

void
tick (conge_ctx* ctx)
{
  if (ctx->keys[CONGE_ESC])
    ctx->exit = 1;
}

int
main (void)
{
  conge_ctx* ctx = conge_init ();
  conge_run (ctx, tick, 30);
  conge_free (ctx);

  return 0;
}
