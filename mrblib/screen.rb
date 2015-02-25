class Screen
  SCREEN_X_SIZE = 20
  SCREEN_Y_SIZE = 7

  class << self
    attr_accessor :x, :y
  end

  def self.fresh
    self.x = 0
    self.y = 0
  end

  def add_y(value)
    self.y += value
    self.x = 0
    if self.y > (SCREEN_Y_SIZE - 1)
       self.y = 0
    end
  end

  self.fresh
end
