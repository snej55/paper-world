# requires pygame/pygame-ce installed (via pip3, distro's package manager or something else)

import pygame, sys, time, json, math, random

TILE_SIZE = 8
CHUNK_SIZE = 9
LEVEL_WIDTH = 14
LEVEL_HEIGHT = 7

# saved levels store tile types as integers
CONVERT_TYPES = {
    0: 'grass',
    1: 'rock', 
    2: 'spike',
    3: 'grass_key',
    4: 'trees'
}
AUTO_TILE_TYPES = {'grass', 'rock'}
AUTO_TILE_MAP = {'0011': 1, '1011': 2, '1001': 3, '0001': 4, '0111': 5, '1111': 6, '1101': 7, '0101': 8, 
                '0110': 9, '1110': 10, '1100': 11, '0100': 12, '0010': 13, '1010': 14, '1000': 15, '0000': 16}
ENTITIES = {'slime', 'bat', 'turtle'}

class Editor:
    def __init__(self):
        self.path = 'data/maps/0.json'

        self.display = pygame.display.set_mode((1000, 800))
        self.screen = pygame.Surface((500, 400))
        self.dt = 1
        self.last_time = time.time() - 1 / 60
        self.clock = pygame.time.Clock()
        self.running = True

        self.assets = {
            'grass': self.load_tileset(pygame.image.load('data/images/tiles/grass.png').convert()),
            'rock': self.load_tileset(pygame.image.load('data/images/tiles/rock.png').convert()),
            'spike': self.load_tileset(pygame.image.load('data/images/tiles/spike.png').convert()),
            'slime': [pygame.image.load('data/images/entities/slime/thumb.png').convert()],
            'bat': [pygame.image.load('data/images/entities/bat/thumb.png').convert()],
            'turtle': [pygame.image.load('data/images/entities/turtle/thumb.png').convert()],
            'spring': [pygame.image.load('data/images/tiles/spring.png').convert()],
            'grass_key': [pygame.image.load('data/images/tiles/grass_key.png').convert()],
            'trees': self.load_sheet(pygame.image.load('data/images/tiles/trees.png').convert(), [32, 32])
        }
        
        for key in self.assets:
            for surf in self.assets[key]:
                surf.set_colorkey((0, 0, 0))

        self.scroll = pygame.Vector2(0, 0)
        self.controls = {'right': False, 'left': False, 'up': False, 'down': False, 'l_shift': False}

        self.tile_map = {}
        self.off_grid = []
        self.water_list = []
        self.water_rect = None
        self.load(self.path)

        self.click = False
        self.right_click = False

        self.select_surf = pygame.Surface((TILE_SIZE, TILE_SIZE))
        self.select_surf.fill((255, 100, 0))
        self.select_surf.set_alpha(100)

        self.tile_list = list(self.assets)
        self.tile_type = 0
        self.tile_variant = 0

        self.grid = True
        self.water = False

        self.particles = []
    
    def load(self, path):
        f = open(path, 'r')
        data = json.load(f)
        f.close()
        self.tile_map = {}
        self.off_grid = []
        self.water_list = []
        for tile in data['level']['tiles']:
            tile_loc = f"{tile['pos'][0]};{tile['pos'][1]}"
            self.tile_map[tile_loc] = {'type': CONVERT_TYPES[tile['type']], 'variant': tile['variant']}
        for entity in data['level']['entities']:
            self.tile_map[f"{math.floor(entity['pos'][0] / TILE_SIZE)};{math.floor(entity['pos'][1] / TILE_SIZE)}"] = {'type': entity['type'], 'variant': 0}
        for spring in data['level']['springs']:
            self.tile_map[f"{math.floor(spring['pos'][0] / TILE_SIZE)};{math.floor(spring['pos'][1] / TILE_SIZE)}"] = {'type': "spring", 'variant': 0}
        for rect in data['level']['water']:
            self.water_list.append(pygame.Rect(rect[0], rect[1], rect[2], rect[3]))
        self.off_grid.extend(data['level']['off_grid'])
        for tile in self.off_grid:
            tile['type'] = CONVERT_TYPES[tile['type']]
    
    def save(self, path):
        with open(path, 'w') as f:
            tiles = []
            entities = []
            springs = []
            off_grid = []
            for loc in self.tile_map:
                tile_type = 0
                for key in CONVERT_TYPES:
                    if self.tile_map[loc]['type'] == CONVERT_TYPES[key]:
                        tile_type = key
                entity = False
                for entity_type in ENTITIES:
                    if self.tile_map[loc]['type'] == entity_type:
                        entities.append({'type': self.tile_map[loc]['type'], 'pos': [int(c) * TILE_SIZE for c in loc.split(';')]})
                        entity = True
                        break
                if not entity:
                    if self.tile_map[loc]['type'] == "spring":
                        springs.append({'pos': [int(c) * TILE_SIZE for c in loc.split(';')]})
                    else:
                        tiles.append({'pos': [int(c) for c in loc.split(';')], 'type': tile_type, 'variant': self.tile_map[loc]['variant']})
            for tile in self.off_grid:
                tile_type = 0
                for key in CONVERT_TYPES:
                    if tile['type'] == CONVERT_TYPES[key]:
                        tile_type = key
                off_grid.append({'pos': tile['pos'], 'type': tile_type, 'variant': tile['variant']});
            json.dump({'level': {'tiles': tiles, 'entities': entities, 'springs': springs, 'off_grid': off_grid, 'water': [[rect.x, rect.y, rect.w, rect.h] for rect in self.water_list]}}, f)
            f.close()

    def load_tileset(self, sheet):
        tiles = []
        for y in range(4):
            for x in range(4):
                tile_surf = pygame.Surface((TILE_SIZE, TILE_SIZE))
                tile_surf.blit(sheet, (-x * TILE_SIZE, -y * TILE_SIZE))
                tile_surf.set_colorkey((0, 0, 0))
                tiles.append(tile_surf.copy())
        return tiles

    def load_sheet(self, sheet, tile_size):
        tiles = []
        for x in range(math.floor(sheet.get_width() / tile_size[0])):
            tile_surf = pygame.Surface(tile_size)
            tile_surf.blit(sheet, (-x * tile_size[0], 0))
            tile_surf.set_colorkey((0, 0, 0))
            tiles.append(tile_surf.copy())
        return tiles
    
    def auto_tile(self):
        for loc in self.tile_map:
            tile = self.tile_map[loc]
            aloc = ''
            tile_pos = [int(i) * TILE_SIZE for i in loc.split(';')]
            for shift in [(-1, 0), (0, -1), (1, 0), (0, 1)]:
                check_loc = str(math.floor(tile_pos[0] / TILE_SIZE) + shift[0]) + ';' + str(math.floor(tile_pos[1] / TILE_SIZE) + shift[1])
                if check_loc in self.tile_map:
                    if self.tile_map[check_loc]['type'] in AUTO_TILE_TYPES:
                        aloc += '1'
                    else:
                        aloc += '0'
                else:
                    aloc += '0'
            if tile['type'] in AUTO_TILE_TYPES:
                tile['variant'] = AUTO_TILE_MAP[aloc] - 1

    def close(self):
        self.running = False
        pygame.quit()
        sys.exit()
    
    def update(self):
        self.scroll.x += (int(self.controls['right']) - int(self.controls['left'])) * 2 * self.dt
        self.scroll.y += (int(self.controls['down']) - int(self.controls['up'])) * 2 * self.dt

        mouse_pos = pygame.mouse.get_pos()
        mouse_pos = [math.floor((mouse_pos[0] / 2 + self.scroll.x) / TILE_SIZE), math.floor((mouse_pos[1] / 2 + self.scroll.y) / TILE_SIZE)]

        if not self.water:
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
                        for y in range(TILE_SIZE):
                            for x in range(TILE_SIZE):
                                try:
                                    angle = random.random() * math.pi * 2
                                    #self.particles.append([[mouse_pos[0] * TILE_SIZE + random.random() * TILE_SIZE, mouse_pos[1] * TILE_SIZE + random.random() * TILE_SIZE], [random.random() * 2 - 1, random.random() * 4 - 3], 0, random.choice([(96, 174, 123), (60, 107, 100), (31, 36, 75), (101, 64, 83), (168, 96, 93), (209, 166, 126), (246, 231, 156), (182, 207, 142)])])
                                    self.particles.append([[mouse_pos[0] * TILE_SIZE + x, mouse_pos[1] * TILE_SIZE + y], [math.sin(angle), math.cos(angle)], 0, self.assets[self.tile_map[tile_loc]['type']][self.tile_map[tile_loc]['variant']].get_at((x, y))])
                                except IndexError:
                                    pass
                        del self.tile_map[tile_loc]
        else:
            if self.click and self.grid:
                mouse_pos[0] *= TILE_SIZE
                mouse_pos[1] *= TILE_SIZE
                if mouse_pos[0] < self.water_rect.x:
                    self.water_rect.w = self.water_rect.x - mouse_pos[0]
                    self.water_rect.x = mouse_pos[0]
                else:
                    self.water_rect.w = mouse_pos[0] - self.water_rect.x
                if mouse_pos[1] < self.water_rect.y:
                    self.water_rect.h = self.water_rect.y - mouse_pos[1]
                    self.water_rect.y = mouse_pos[1]
                else:
                    self.water_rect.h = mouse_pos[1] - self.water_rect.y
                pygame.draw.rect(self.screen, (200, 200, 200, 200), (self.water_rect.x - self.scroll.x, self.water_rect.y - self.scroll.y, self.water_rect.w, self.water_rect.h))
            if self.right_click:
                mouse_pos = pygame.mouse.get_pos()
                mouse_pos = [math.floor((mouse_pos[0] / 2 + self.scroll.x)), math.floor((mouse_pos[1] / 2 + self.scroll.y))]
                for i, rect in sorted(enumerate(self.water_list), reverse=True):
                    if rect.collidepoint(mouse_pos[0], mouse_pos[1]):
                        self.water_list.pop(i)
        for rect in self.water_list:
            pygame.draw.rect(self.screen, (0, 100, 255, 255), (rect.x - self.scroll.x, rect.y - self.scroll.y, rect.w, rect.h))
                
        
        for i, particle in sorted(enumerate(self.particles), reverse=True):
            particle[0][0] += particle[1][0] * self.dt
            particle[0][1] += particle[1][1] * self.dt
            particle[1][1] += 0.2 * self.dt
            particle[2] += 0.2 * self.dt
            if particle[2] > 10:
                self.particles.pop(i)
            else:
                color = pygame.Color(particle[3])
                self.screen.set_at((particle[0][0] - self.scroll.x, particle[0][1] - self.scroll.y), color)

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
        for tile in self.off_grid: # tile: [pos, type, variant] absolute pos
            self.screen.blit(self.assets[tile['type']][tile['variant']], (tile['pos'][0] - self.scroll.x, tile['pos'][1] - self.scroll.y))
    
        mouse_pos = pygame.mouse.get_pos()
        if self.grid:
            mouse_pos = [math.floor((mouse_pos[0] / 2 + self.scroll.x) / TILE_SIZE), math.floor((mouse_pos[1] / 2 + self.scroll.y) / TILE_SIZE)]
            self.screen.blit(self.select_surf, (mouse_pos[0] * TILE_SIZE - self.scroll.x, mouse_pos[1] * TILE_SIZE - self.scroll.y))
            if not self.right_click:
                self.screen.blit(self.assets[self.tile_list[self.tile_type]][self.tile_variant], (mouse_pos[0] * TILE_SIZE - self.scroll.x, mouse_pos[1] * TILE_SIZE - self.scroll.y))
        else:
            mouse_pos = [math.floor(mouse_pos[0] / 2 + self.scroll.x), math.floor(mouse_pos[1] / 2 + self.scroll.y)]
            self.screen.blit(self.select_surf, (mouse_pos[0] - self.scroll.x, mouse_pos[1] - self.scroll.y))
            if not self.right_click:
                self.screen.blit(self.assets[self.tile_list[self.tile_type]][self.tile_variant], (mouse_pos[0] - self.scroll.x, mouse_pos[1] - self.scroll.y))

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
                    if event.key == pygame.K_t:
                        self.auto_tile()
                    if event.key == pygame.K_o:
                        self.save(self.path)
                    if event.key == pygame.K_g:
                        self.grid = not self.grid
                    if event.key == pygame.K_w:
                        self.water = not self.water
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
                        if self.water:
                            mouse_pos = pygame.mouse.get_pos()
                            mouse_pos = [math.floor((mouse_pos[0] / 2 + self.scroll.x) / TILE_SIZE), math.floor((mouse_pos[1] / 2 + self.scroll.y) / TILE_SIZE)]
                            self.water_rect = pygame.Rect(mouse_pos[0] * TILE_SIZE, mouse_pos[1] * TILE_SIZE, TILE_SIZE, TILE_SIZE)
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
                    if not self.grid:
                        mouse_pos = pygame.mouse.get_pos()
                        mouse_pos = [math.floor(mouse_pos[0] / 2 + self.scroll.x), math.floor(mouse_pos[1] / 2 + self.scroll.y)]
                        if not self.water:
                            if self.click:
                                if 0 <= mouse_pos[0] < LEVEL_WIDTH * CHUNK_SIZE * TILE_SIZE and 0 <= mouse_pos[1] < LEVEL_HEIGHT * CHUNK_SIZE * TILE_SIZE:
                                    self.off_grid.append({'pos': mouse_pos, 'type': self.tile_list[self.tile_type], 'variant': self.tile_variant}) 
                            if self.right_click:
                                for i, tile in sorted(enumerate(self.off_grid), reverse=True):
                                    tile_img = self.assets[tile['type']][tile['variant']];
                                    tile_rect = pygame.Rect(mouse_pos[0], mouse_pos[1], tile_img.get_width(), tile_img.get_height())
                                    if tile_rect.collidepoint(mouse_pos[0], mouse_pos[1]):
                                        self.off_grid.pop(i)
                if event.type == pygame.MOUSEBUTTONUP:
                    if event.button == 1:
                        self.click = False
                        if self.water:
                            self.water_list.append(self.water_rect.copy())
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
