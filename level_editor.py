import pygame, sys, time, json, math

TILE_SIZE = 8
CHUNK_SIZE = 9
LEVEL_WIDTH = 14
LEVEL_HEIGHT = 7

# saved levels store tile types as integers
CONVERT_TYPES = {
    0: 'grass',
    1: 'rock'
}
AUTO_TILE_TYPES = {'grass', 'rock'}

class Editor:
    def __init__(self):
        self.display = pygame.display.set_mode((1000, 800))
        self.screen = pygame.Surface((500, 400))
        self.dt = 1
        self.last_time = time.time() - 1 / 60
        self.clock = pygame.time.Clock()
        self.running = True

        self.assets = {
            'grass': self.load_tileset(pygame.image.load('data/images/tiles/grass.png').convert()),
            'rock': self.load_tileset(pygame.image.load('data/images/tiles/rock.png').convert())
        }

        self.scroll = pygame.Vector2(0, 0)
        self.controls = {'right': False, 'left': False, 'up': False, 'down': False, 'l_shift': False}

        self.tile_map = {}
        self.load('data/maps/0.json')

        self.click = False
        self.right_click = False

        self.select_surf = pygame.Surface((TILE_SIZE, TILE_SIZE))
        self.select_surf.fill((255, 100, 0))
        self.select_surf.set_alpha(100)

        self.tile_list = list(self.assets)
        self.tile_type = 0
        self.tile_variant = 0

        self.grid = True
    
    def load(self, path):
        f = open(path, 'r')
        data = json.load(f)
        f.close()
        self.tile_map = {}
        for tile in data['level']['tiles']:
            tile_loc = f"{tile['pos'][0]};{tile['pos'][1]}"
            self.tile_map[tile_loc] = {'type': CONVERT_TYPES[tile['type']], 'variant': tile['variant']}
    
    def load_tileset(self, sheet):
        tiles = []
        for y in range(4):
            for x in range(4):
                tile_surf = pygame.Surface((TILE_SIZE, TILE_SIZE))
                tile_surf.blit(sheet, (-x * TILE_SIZE, -y * TILE_SIZE))
                tile_surf.set_colorkey((0, 0, 0))
                tiles.append(tile_surf.copy())
        return tiles
    
    def close(self):
        self.running = False
        pygame.quit()
        sys.exit()
    
    def update(self):
        self.scroll.x += (int(self.controls['right']) - int(self.controls['left'])) * 2 * self.dt
        self.scroll.y += (int(self.controls['down']) - int(self.controls['up'])) * 2 * self.dt

        mouse_pos = pygame.mouse.get_pos()
        mouse_pos = [math.floor((mouse_pos[0] / 2 + self.scroll.x) / TILE_SIZE), math.floor((mouse_pos[1] / 2 + self.scroll.y) / TILE_SIZE)]

        if self.click and self.grid:
            if 0 <= mouse_pos[0] < LEVEL_WIDTH * CHUNK_SIZE and 0 <= mouse_pos[1] < LEVEL_HEIGHT * CHUNK_SIZE:
                tile_loc = f"{mouse_pos[0]};{mouse_pos[1]}"
                if tile_loc in self.tile_map:
                    if self.tile_map[tile_loc]['type'] == self.tile_list[self.tile_type] and self.tile_map[tile_loc]['variant'] == self.tile_variant:
                        pass
                    else:
                        self.tile_map[tile_loc] = {'type': self.tile_list[self.tile_type], 'variant': self.tile_variant}
                else:
                    self.tile_map[tile_loc] = {'type': self.tile_list[self.tile_type], 'variant': self.tile_variant}
        if self.right_click and self.grid:
            if 0 <= mouse_pos[0] < LEVEL_WIDTH * CHUNK_SIZE and 0 <= mouse_pos[1] < LEVEL_HEIGHT * CHUNK_SIZE:
                tile_loc = f"{mouse_pos[0]};{mouse_pos[1]}"
                if tile_loc in self.tile_map:
                    del self.tile_map[tile_loc]

    def draw_tiles(self):
        for x in range(math.floor(self.scroll.x / TILE_SIZE), math.floor((self.scroll.x + self.screen.get_width()) // TILE_SIZE + 1)):
            for y in range(math.floor(self.scroll.y / TILE_SIZE), math.floor((self.scroll.y + self.screen.get_height()) // TILE_SIZE + 1)):
                loc = str(x) + ';' + str(y)
                if loc in self.tile_map:
                    self.screen.blit(self.assets[self.tile_map[loc]["type"]][self.tile_map[loc]['variant']], (x * TILE_SIZE - self.scroll.x, y * TILE_SIZE - self.scroll.y))
    
    def draw_tile_grid(self, scroll, size, color):
        tile_size = [TILE_SIZE * size[0], TILE_SIZE * size[1]]
        length = math.ceil(self.screen.get_width() / tile_size[0]) + 2
        height = math.ceil(self.screen.get_height() / tile_size[1]) + 2
        for x in range(length):
            pygame.draw.line(self.screen, color, ((x - 1) * tile_size[0] - (scroll[0] % tile_size[0]), 0), ((x - 1) * tile_size[0] - (scroll[0] % tile_size[0]), self.screen.get_height()))
        for y in range(height):
            pygame.draw.line(self.screen, color, (0, (y - 1) * tile_size[1] - (scroll[1] % tile_size[1])), (self.screen.get_width(), (y - 1) * tile_size[1] - (scroll[1] % tile_size[1])))
    
    def draw_grid(self):
        self.draw_tile_grid(self.scroll, [1, 1], (50, 50, 50))
        self.draw_tile_grid(self.scroll, [CHUNK_SIZE, CHUNK_SIZE], (100, 100, 255))
        pygame.draw.line(self.screen, (255, 255, 255), (-self.scroll.x, -self.scroll.y), (LEVEL_WIDTH * CHUNK_SIZE * TILE_SIZE - self.scroll.x, -self.scroll.y), 1)
        pygame.draw.line(self.screen, (255, 255, 255), (-self.scroll.x, -self.scroll.y), (-self.scroll.x, LEVEL_HEIGHT * CHUNK_SIZE * TILE_SIZE - self.scroll.y), 1)
        pygame.draw.line(self.screen, (255, 255, 255), (LEVEL_WIDTH * CHUNK_SIZE * TILE_SIZE - self.scroll.x, -self.scroll.y), (LEVEL_WIDTH * CHUNK_SIZE * TILE_SIZE - self.scroll.x, LEVEL_HEIGHT * CHUNK_SIZE * TILE_SIZE - self.scroll.y), 1)
        pygame.draw.line(self.screen, (255, 255, 255), (-self.scroll.x, LEVEL_HEIGHT * CHUNK_SIZE * TILE_SIZE - self.scroll.y), (LEVEL_WIDTH * CHUNK_SIZE * TILE_SIZE - self.scroll.x, LEVEL_HEIGHT * CHUNK_SIZE * TILE_SIZE - self.scroll.y), 1)

    def draw(self):
        self.draw_grid()
        self.draw_tiles()
    
        mouse_pos = pygame.mouse.get_pos()
        mouse_pos = [math.floor((mouse_pos[0] / 2 + self.scroll.x) / TILE_SIZE), math.floor((mouse_pos[1] / 2 + self.scroll.y) / TILE_SIZE)]
        self.screen.blit(self.select_surf, (mouse_pos[0] * TILE_SIZE - self.scroll.x, mouse_pos[1] * TILE_SIZE - self.scroll.y))
        self.screen.blit(self.assets[self.tile_list[self.tile_type]][self.tile_variant], (mouse_pos[0] * TILE_SIZE - self.scroll.x, mouse_pos[1] * TILE_SIZE - self.scroll.y))

    def run(self):
        while self.running:
            self.dt = time.time() - self.last_time
            self.dt *= 60
            self.last_time = time.time()
            self.screen.fill((0, 0, 0))
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    self.close()
                if event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_ESCAPE:
                        self.close()
                    if event.key == pygame.K_RIGHT:
                        self.controls['right'] = True
                    if event.key == pygame.K_LEFT:
                        self.controls['left'] = True
                    if event.key == pygame.K_UP:
                        self.controls['up'] = True
                    if event.key == pygame.K_DOWN:
                        self.controls['down'] = True
                    if event.key == pygame.K_LSHIFT:
                        self.controls['l_shift'] = True
                if event.type == pygame.KEYUP:
                    if event.key == pygame.K_RIGHT:
                        self.controls['right'] = False
                    if event.key == pygame.K_LEFT:
                        self.controls['left'] = False
                    if event.key == pygame.K_UP:
                        self.controls['up'] = False
                    if event.key == pygame.K_DOWN:
                        self.controls['down'] = False
                    if event.key == pygame.K_LSHIFT:
                        self.controls['l_shift'] = False
                if event.type == pygame.MOUSEBUTTONDOWN:
                    if event.button == 1:
                        self.click = True
                    if event.button == 3:
                        self.right_click = True
                    if self.controls['l_shift']:
                        if event.button == 4:
                            self.tile_variant = (self.tile_variant - 1) % len(self.assets[self.tile_list[self.tile_type]])
                        if event.button == 5:
                            self.tile_variant = (self.tile_variant + 1) % len(self.assets[self.tile_list[self.tile_type]])
                    else:
                        if event.button == 4:
                            self.tile_type = (self.tile_type - 1) % len(self.tile_list)
                            self.tile_variant = 0
                        if event.button == 5:
                            self.tile_type = (self.tile_type + 1) % len(self.tile_list)
                            self.tile_variant = 0
                if event.type == pygame.MOUSEBUTTONUP:
                    if event.button == 1:
                        self.click = False
                    if event.button == 3:
                        self.right_click = False
            self.update()
            self.draw()
            pygame.transform.scale_by(self.screen, 2.0, self.display)
            pygame.display.set_caption(f'LEVEL EDITOR at {self.clock.get_fps() :.1f} FPS!')
            pygame.display.flip()
            self.clock.tick(60)

if __name__ == '__main__':
    Editor().run()
