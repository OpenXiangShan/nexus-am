#include <am.h>
#include <imgui.h>

//#include <stdio.h>

float min(float a, float b, float c) {
  if (a < b && a < c) return a;
  if (b < a && b < c) return b;
  return c;
}
float max(float a, float b, float c) {
  if (a > b && a > c) return a;
  if (b > a && b > c) return b;
  return c;
}

static void render_triangle(const ImDrawVert *a, const ImDrawVert *b, const ImDrawVert *c) {
  float minx = min(a->pos.x, b->pos.x, c->pos.x);
  float miny = min(a->pos.y, b->pos.y, c->pos.y);
  float maxx = max(a->pos.x, b->pos.x, c->pos.x);
  float maxy = max(a->pos.y, b->pos.y, c->pos.y);
  for (int x = minx; x <= maxx; x ++)
    for (int y = miny; y <= maxy; y ++) {
      //int x1 = x - 500;
      int x1 = x;
      if (x1 >= 0 && x1 < 640 && y >= 0 && y < 480) {
        _draw_p(x1, y, c->col);
      }
    }
}

void render(ImDrawData *draw_data) {
//  printf("=== Render ===\n");
  for (int n = 0; n < draw_data->CmdListsCount; n ++) {
    const ImDrawList *cmd_list = draw_data->CmdLists[n];
    const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
    const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;
    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i ++) {
      const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
      if (pcmd->UserCallback) {
        pcmd->UserCallback(cmd_list, pcmd);
      } else {
        int nt = pcmd->ElemCount;
//        printf("Render texture %ld, %d triangles\n", (ulong)pcmd->TextureId, nt);
        assert(nt % 3 == 0);
        for (int i = 0; i < nt; i += 3) {
          const ImDrawVert *a = vtx_buffer + idx_buffer[i];
          const ImDrawVert *b = vtx_buffer + idx_buffer[i+1];
          const ImDrawVert *c = vtx_buffer + idx_buffer[i+2];
          render_triangle(a, b, c);
        }
//        printf("\n");
      }
    }
  }
//  printf("Render %d elemts.\n", draw_data->CmdListsCount);
  _draw_sync();
}

int main() {
  _ioe_init();

  ImGuiIO &io = ImGui::GetIO();

  unsigned char *pixels;
  int width, height;
  io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

  io.RenderDrawListsFn = render;
  //ImVec4 clear_color = ImColor(114, 144, 154);
  int mx = 10, my = 10;
  while (1) {
    mx += 1; my += 1;
    if (mx > 400) { mx = my = 10; }

    io.MousePos = ImVec2(float(mx), float(my));
    io.MouseDrawCursor = 1;
    io.DisplaySize = ImVec2(_screen.width, _screen.height);
    io.DisplayFramebufferScale = ImVec2(1.0, 1.0);
    io.DeltaTime = 1.0f / 60.0f;
    ImGui::NewFrame();
    bool show_test_window = true;
    {
      ImGui::SetNextWindowPos(ImVec2(40, 20), ImGuiSetCond_FirstUseEver);
      ImGui::Begin("New Window");
      ImGui::Text("Hello, world!");
      ImGui::End();
    }

    ImGui::ShowTestWindow(&show_test_window);

    for (int i = 0; i < _screen.width; i ++)
      for (int j = 0; j < _screen.height; j ++) {
        _draw_p(i, j, (114<<16) | (144<<8) | 154);
      }
    ImGui::Render();
  }
}
