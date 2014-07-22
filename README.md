# MRuby-pax

MRuby on pax, implement the entire dependencies to development and deploy for PAX D200.

## Setup

Initialize and update submodules

	$ git submodule init
	$ git submodule update

## Platform Info

Details about the environment

1. Support only 16 bytes file names.
2. Support only 255 files.
3. Don't support rename, pipe, f* functions(fseek, fopen, fread).
	
	
## Usage
Rake tasks availables on Vagrant:

	rake pax:clean          # Clobber/Clean PAX
	rake pax:compile        # Compile PAX
	rake pax:musl:clean     # Clean Musl libc
	rake pax:musl:compile   # Compile Musl libc
	rake pax:musl:setup     # Setup musl ENV
	rake pax:setup          # Setup PAX ENV
	
Rake tasks availables on Visual Studio Commnad Prompt

	rake pax:mruby:clean    # Clean MRuby
	rake pax:mruby:compile  # Compile MRuby and generate libmruby.a
	rake pax:mruby:setup    # Setup

## Contributing

1. Fork it
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -am 'Added some feature'`)
4. Push to the branch (`git push origin my-new-feature`)
5. Create new Pull Request


Adicionar assunto sobre File, mrubygem io.
Adicionar unpack pack
Escrever mais samples, buscat no gist