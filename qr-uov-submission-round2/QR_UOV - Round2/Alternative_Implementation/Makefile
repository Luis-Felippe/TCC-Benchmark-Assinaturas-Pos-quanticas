# platform: portable64 ref avx2 avx512
platform := avx2
subdirs :=\
  qruov1q127L3v156m54 \
  qruov3q127L3v228m78 \
  qruov5q127L3v306m105

.PHONY: all clean $(subdirs)

all: $(subdirs)

$(subdirs): qruov_config.src
	mkdir -p $@/$(platform)a
	grep $@ qruov_config.src > $@/$(platform)a/qruov_config.txt
	sh -c "cd $@/$(platform)a ; ln -s ../../$(platform)/* . || true"
	$(MAKE) -C $@/$(platform)a qruov_bench

clean:
	rm -rf $(subdirs)