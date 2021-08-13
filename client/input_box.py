import pygame

COLOR_INACTIVE = (138, 138, 138)
COLOR_ACTIVE = (255, 255, 255)

class InputBox:
    def __init__(self, font, x, y, w, h, text=''):
        self.rect = pygame.Rect(x, y, w, h)
        self.color = COLOR_INACTIVE
        self.text = text
        self.font = font
        self.text_surface = self.font.render(text, 1, self.color)
        self.active = False

    def handle_event(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN:
            if self.rect.collidepoint(event.pos):
                self.active = not self.active
            else:
                self.active = False

        if self.active:
            self.color = COLOR_ACTIVE
        else:
            self.color = COLOR_INACTIVE

        message = ''

        if event.type == pygame.KEYDOWN:
            if self.active:
                if event.key == pygame.K_RETURN:
                    message = self.text
                    self.text = ''
                elif event.key == pygame.K_BACKSPACE:
                    self.text = self.text[:-1]
                else:
                    self.text += event.unicode

                self.text_surface = self.font.render(self.text, 1, self.color)

        return message

    def update(self):
        for event in pygame.event.get():
            self.handle_event(event)

        width = max(300, self.text_surface.get_width()+10)
        self.rect.w = width

    def draw(self, screen):
        screen.blit(self.text_surface, (self.rect.x + 5, self.rect.y + 5))
        pygame.draw.rect(screen, self.color, self.rect, 2)