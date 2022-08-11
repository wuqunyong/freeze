import os
import sys
import logging
import shutil
import time

logging.basicConfig(level=logging.INFO, filename="MergeFiles.log", filemode="w", format="%(asctime)s | %(levelname)s | %(message)s")


def TraversalDir(listObj, path):
    if os.path.exists(path):
        files = os.listdir(path)
        for file in files:
            m = os.path.join(path, file)
            if os.path.isdir(m):
                TraversalDir(listObj, m)
            elif os.path.isfile(m):
                remainder, filename = os.path.split(m)
                if filename.startswith("AutoMergeFiles"):
                    continue
                if m.endswith(".sql"):
                    listObj.append(m)

def main(argc, argv ):
    if argc < 2 :
        logging.error("invalid argc:{}".format(argc))
        return
    sDir = argv[1]
    logging.info("dir:{}".format(sDir))
    filelist = []
    TraversalDir(filelist, sDir)
    logging.info("files:{}".format(filelist))

    sMergeComment = """
-- --------------------------------------------------------
-- dir:{}
-- date:{}
-- --------------------------------------------------------

"""


    sFileHead = """
    
    
-- --------------------------------------------------------
-- filename:{}
-- --------------------------------------------------------
"""
    sData = time.strftime("%Y%m%d%H%M%S", time.localtime())

    sMergeName = "AutoMergeFiles_" + sData + ".sql"

    with open(sMergeName, "w") as fdst:
        sData = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())
        fdst.write(sMergeComment.format(sDir, sData))

    for filepath in filelist:
        with open(sMergeName, "a") as fdst:
                fdst.write(sFileHead.format(filepath))

        with open(sMergeName, "ab") as fdst:
            with open(filepath, "rb") as fsrc:
                shutil.copyfileobj(fsrc, fdst)


if __name__ == "__main__":
    logging.info("args:{}".format(sys.argv))
    argc = len(sys.argv)
    main(argc, sys.argv)
