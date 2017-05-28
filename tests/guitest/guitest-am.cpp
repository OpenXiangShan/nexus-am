#include <am.h>
#include <imgui.h>

#include <stdio.h>


static unsigned char *texture;
static int t_width, t_height;

float min(float a, float b, float c) {
  if (a <= b && a <= c) return a;
  if (b <= a && b <= c) return b;
  return c;
}
float max(float a, float b, float c) {
  if (a >= b && a >= c) return a;
  if (b >= a && b >= c) return b;
  return c;
}

template<typename T>
void swap(T &a, T &b) {
  T tmp = a;
  a = b;
  b = tmp;
}

int at_left(float x, float y, ImVec2 p, ImVec2 q) {
  x += 0.49; y += 0.49;
  if (p.y > q.y) {
    swap(p, q);
  }
  if (!(p.y <= y && y <= q.y)) return 0;
  return (x - p.x) * (q.y - p.y) - (y - p.y) * (q.x - p.x) <= 0;
}

static u32 *fb;

static void render_triangle(const ImDrawCmd *pcmd, const ImDrawVert *a, const ImDrawVert *b, const ImDrawVert *c) {
  float minx = min(a->pos.x, b->pos.x, c->pos.x);
  float maxx = max(a->pos.x, b->pos.x, c->pos.x);
  float miny = min(a->pos.y, b->pos.y, c->pos.y);
  float maxy = max(a->pos.y, b->pos.y, c->pos.y);

  float minu = min(a->uv.x, b->uv.x, c->uv.x);
  float maxu = max(a->uv.x, b->uv.x, c->uv.x);
  float minv = min(a->uv.y, b->uv.y, c->uv.y);
  float maxv = max(a->uv.y, b->uv.y, c->uv.y);

  float du = (maxu - minu) / (maxx - minx);
  float dv = (maxv - minv) / (maxy - miny);

  auto &clip = pcmd->ClipRect;

  float u = minu;
  for (int x = minx; x <= maxx; x ++) {
    float v = minv;
    for (int y = miny; y <= maxy; y ++) {
      if (x >= 0 && x < _screen.width && y >= 0 && y < _screen.height &&
          clip.x <= x && x <= clip.z && 
          clip.y <= y && y <= clip.w
      ) {
        int ns = 
          at_left(x, y, a->pos, b->pos) +
          at_left(x, y, b->pos, c->pos) +
          at_left(x, y, c->pos, a->pos);

        if (ns & 1) {
          int tx = u * t_width;
          int ty = v * t_height;
          u8 alpha = texture[tx + ty * t_width];
          if (alpha > 0) {
            int idx = x + y * _screen.width;
            u32 old = fb[idx];
            u32 col = a->col & 0xffffff;
            fb[idx] = ((col * alpha + old * (255 - alpha)) / 255) & 0xffffff;
          }
        }
      }
      v += dv;
    }
    u += du;
  }
}

void render(ImDrawData *draw_data) {
  for (int i = 0; i < _screen.width * _screen.height; i ++)
    fb[i] = IM_COL32(114, 144, 154, 255);

//  printf("=== Render ===\n");
  for (int n = 0; n < draw_data->CmdListsCount; n ++) {
    printf("n = %d\n", n);
    const ImDrawList *cmd_list = draw_data->CmdLists[n];
    const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
    const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;
    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i ++) {
      const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
      if (pcmd->UserCallback) {
        pcmd->UserCallback(cmd_list, pcmd);
      } else {
        int nt = pcmd->ElemCount;
        printf("Render texture %lx, %d triangles\n", (ulong)pcmd->TextureId, nt/3);
        assert(nt % 3 == 0);
        for (int i = 0; i < nt; i += 3) {
          const ImDrawVert *a = vtx_buffer + idx_buffer[i];
          const ImDrawVert *b = vtx_buffer + idx_buffer[i+1];
          const ImDrawVert *c = vtx_buffer + idx_buffer[i+2];
          render_triangle(pcmd, a, b, c); // a, b, c);
        }
      }
      idx_buffer += pcmd->ElemCount;
    }
  }
  for (int i = 0; i < _screen.width * _screen.height; i ++) {
    u32 p = fb[i];
    fb[i] = ((p >> 16) & 0xff) | (((p >> 8) & 0xff) << 8) | ((p & 0xff) << 16);
  }
  _draw_f(fb);
  _draw_sync();
}

int main() {
  _ioe_init();

  ImGuiIO &io = ImGui::GetIO();
  fb = new u32[_screen.width * _screen.height];

  io.Fonts->GetTexDataAsAlpha8(&texture, &t_width, &t_height);
  io.Fonts->TexID = texture;

  io.RenderDrawListsFn = render;
  int mx = 60, my = 150;
  static int frames = 0;
  bool click = true;
  while (1) {
    frames ++;
    mx += 3; my += 2;
    if (mx > 400) { mx = my = 10; }

    io.MousePos = ImVec2(float(mx), float(my));
    io.MouseDrawCursor = 1;
    io.DisplaySize = ImVec2(_screen.width, _screen.height);
    io.DisplayFramebufferScale = ImVec2(1.0, 1.0);
    io.DeltaTime = 1.0f / 60.0f;
    ImGui::NewFrame();

    if (click && frames % 5 == 0) {
      click = false;
      io.MouseDown[0] = 1;
    }
    if (frames % 5 == 1) {
      io.MouseDown[0] = 0;
    }
    bool show_test_window = true;
    ImGui::SetNextWindowPos(ImVec2(60, 10), ImGuiSetCond_FirstUseEver);
    ImGui::ShowTestWindow(&show_test_window);

    ulong t1 = _uptime();
    ImGui::Render();
    ulong used = _uptime() - t1;
    printf("Render time: %ld ms\n", used);
  }
}
