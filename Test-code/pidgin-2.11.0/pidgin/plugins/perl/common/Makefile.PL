use 5.006;
use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence the contents
# of the Makefile that is written.
WriteMakefile(
    'NAME'             => 'Pidgin',
    'VERSION_FROM'     => './Pidgin.pm', # finds $VERSION
    ($] >= 5.005 ? ## Add these new keywords supported since 5.005
      ('ABSTRACT_FROM' => './Pidgin.pm', # finds $ABSTRACT
       'AUTHOR'        => 'Pidgin <http://pidgin.im/>') :  ()),
    'DEFINE'        => '-Wall  -DPURPLE_DISABLE_DEPRECATED -DPIDGIN_DISABLE_DEPRECATED -DFINCH_DISABLE_DEPRECATED -DGNT_DISABLE_DEPRECATED -Waggregate-return -Wcast-align -Wdeclaration-after-statement -Wendif-labels -Werror-implicit-function-declaration -Wextra -Wno-unused-parameter -Wformat-security -Werror=format-security -Winit-self -Wmissing-declarations -Wmissing-noreturn -Wmissing-prototypes -Wpointer-arith -Wundef -Wp,-D_FORTIFY_SOURCE=2',
    'dynamic_lib'       => { 'OTHERLDFLAGS' => '' },
    'INC'           => '-I. -I. -I../../../.. -I../../../../libpurple -I../../../../pidgin -pthread -I/usr/include/gtk-2.0 -I/usr/lib/i386-linux-gnu/gtk-2.0/include -I/usr/include/gio-unix-2.0/ -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pixman-1 -I/usr/include/libpng16 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/libpng16 -I/usr/include/pango-1.0 -I/usr/include/harfbuzz -I/usr/include/pango-1.0 -I/usr/include/glib-2.0 -I/usr/lib/i386-linux-gnu/glib-2.0/include -I/usr/include/freetype2',
    'OBJECT'        => '$(O_FILES)', # link all the C files too
    'TYPEMAPS'      => ["../../../../libpurple/plugins/perl/common/typemap"],
#    'OPTIMIZE'      => '-g', # For debugging.
    'INSTALLDIRS'          => 'vendor',
    'INSTALL_BASE'         => '$(prefix)',
    'INSTALLVENDORARCH'    => '$(libdir)/pidgin/perl',
    'INSTALLVENDORMAN3DIR' => '$(mandir)/man3',
    'macro'                => {
        'prefix'      => '/media/newdrive/llvm3.6/Test-code/pidgin-2.11.0/installed',
        'exec_prefix' => '${prefix}',
        'libdir'      => '${exec_prefix}/lib',
        'mandir'      => '${datarootdir}/man',
        'datarootdir' => '${prefix}/share',
    },
);
