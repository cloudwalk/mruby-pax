class PAX

  # TODO Scalone maybe should use IO class to abstract this
  class ICCard
    ICC_USER_SLOT  = 0
    ICC_SAM1_SLOT  = 2
    ICC_SAM2_SLOT  = 3
    ICC_SAM3_SLOT  = 4
    ICC_SAM4_SLOT  = 5

    SCI_TO_WARNING = 1
    SCI_IDLE       = 0
    SCI_SUCCESS    = 0

    STATUS_OPEN  = :open
    STATUS_CLOSE = :close
    STATUS_ERROR = :error
    STATUS_IDLE  = :idle

    class << self
      attr_accessor :smart_card
    end

    attr_reader :slot, :status, :ret

    # TODO Scalone you cannot open twice
    def self.smart_card_init
      unless self.smart_card && self.smart_card.open?
        self.smart_card = PAX::ICCard.new(ICC_USER_SLOT)
      end
      self.smart_card
    end

    def initialize(slot)
      @slot = slot
      # During the EMV process we should avoid open because the handle managed
      # by EMV Kernel.
      # self.open
    end

    def open
      @ret = PAX::ICCard.open(@slot)
      if (@ret == SCI_SUCCESS)
        @status = STATUS_OPEN
      else
        @status = STATUS_ERROR
      end
    end

    def close
      @ret = PAX::ICCard.close(@slot)
      if (@ret == SCI_SUCCESS)
        @status = STATUS_CLOSE
      else
        @status = STATUS_ERROR
      end
    end

    def detected?
      @ret = PAX::ICCard.detect(@slot)
      @ret == SCI_TO_WARNING
    end

    def removed?
      @ret = PAX::ICCard.detect(@slot)
      @ret == SCI_IDLE
    end

    def open?
      @status == STATUS_OPEN
    end

    def closed?
      @status == STATUS_CLOSE || @status == STATUS_ERROR
    end
  end
end
