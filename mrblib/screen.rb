class Screen
  class << self
    attr_accessor :x, :y
  end

  def self.fresh
    self.x = 0
    self.y = 0
  end

  self.fresh
end
