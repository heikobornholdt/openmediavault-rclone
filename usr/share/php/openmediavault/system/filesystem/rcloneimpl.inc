<?php

namespace OMV\System\Filesystem;

use OMV\Exception;
use OMV\ExecException;
use OMV\System\Filesystem\Backend\BackendAbstract;
use OMV\System\MountPoint;
use OMV\System\Process;

class RcloneImpl implements FilesystemInterface, SharedFolderCandidateInterface {

    // /**
    //  * @var string
    //  */
    // private $dataModelPath = 'conf.service.unionfilesystems.filesystem';

    /**
     * @var string
     */
    private $uuid;

    /**
     * @var string
     */
    private $mountPoint;

    /**
     * @var string
     */
    private $label;

    /**
     * @var BackendAbstract
     */
    private $backend;

    /**
     * @var string
     */
    private $type;

    /**
     * @param string          $uuid       The UUID of the file system
     * @param string          $mountPoint The mount point of the file system
     * @param string          $label      The label of the file system
     * @param BackendAbstract $backend    The backend that manages this filesystem
     */
    public function __construct($uuid, $mountPoint, $label, BackendAbstract $backend) {
        $this->backend = $backend;

        $this->label = $label;
        $this->mountPoint = $mountPoint;
        $this->type = $this->backend->getType();
        $this->uuid = $uuid;
    }

    /**
     * Check if the filesystem exists.
     *
     * @return bool
     */
    public function exists() {
        return true;
    }

    /**
     * Get the backend of this filesystem.
     *
     * @return BackendAbstract
     */
    public function getBackend() {
        return $this->backend;
    }

    /**
     * Get the canonical path of a device file.
     *
     * @return string
     */
    public function getCanonicalDeviceFile() {
        return $this->getDeviceFile();
    }

    /**
     * Get the description of the file system.
     *
     * @return string
     */
    public function getDescription() {
        $statistics = $this->getStatistics();

        return sprintf(
            '%s [%s (%d%%) used, %s available]',
            $this->getLabel(),
            binary_format($statistics['used']),
            $statistics['percentage'],
            binary_format($statistics['available'])
        );
    }

    /**
     * Get the device path of the filesystem, e.g. /dev/sdb1.
     *
     * @return string
     */
    public function getDeviceFile() {
        return $this->mountPoint;
    }

    /**
     * Get the device path by UUID (/dev/disk/by-id/xxx).
     *
     * @return string|null
     */
    public function getDeviceFileById() {
        return null;
    }

    /**
     * Get the device path by label (/dev/disk/by-labed/xxx).
     *
     * @return string|null
     */
    public function getDeviceFileByLabel() {
        return null;
    }

    /**
     * Get the device path by UUID (/dev/disk/by-uuid/xxx).
     *
     * @return string|null
     */
    public function getDeviceFileByUuid() {
        return null;
    }

    /**
     * Get all devices that make up the filesystem.
     *
     * @return array
     */
    public function getDeviceFiles() {
        return [$this->getPreferredDeviceFile()];
    }

    /**
     * Get all device file symlinks via udev.
     *
     * @return array
     */
    public function getDeviceFileSymlinks() {
        return [];
    }

    /**
     * Get the filesystem label.
     *
     * @return string
     */
    public function getLabel() {
        return !empty($this->label) ? $this->label : '';
    }

    /**
     * Get the device of the storage device which contains this file system.
     *
     * @return string|false
     */
    public function getParentDeviceFile() {
        return false;
    }

    /**
     * Get a predictable device file.
     *
     * @return string
     */
    public function getPredictableDeviceFile() {
        return $this->getDeviceFile();
    }

    /**
     * Get the special device file to present in the UI instead of the value of
     * the getDeviceFile method.
     *
     * @return string
     */
    public function getPreferredDeviceFile() {
        return $this->getDeviceFile();
    }

    /**
     * Get the size of the device in bytes.
     *
     * @return int
     */
    public function getSize() {
        return 0;
    }

    /**
     * NOTE: Not part of IFilesystem but needed to display statistics in the
     *       filesystem grid.
     *
     * Get statistics from a mounted filesystem.
     *
     * @return array|bool The filesystem statistics if successful, otherwise
     *                    false. The following fields are included: devicefile,
     *                    type, blocks, size, used, available, percentage and
     *                    mountpoint. Please note, the fields size, used and
     *                    available are strings and their unit is B (bytes)
     *
     * @throws ExecException
     */
    public function getStatistics() {
        $cmd = new Process('df', '-PT', escapeshellarg($this->mountPoint));
        $cmd->setRedirect2to1();
        $cmd->execute($output);

        $result = false;

        foreach ($output as $line) {
            $matches = preg_split('/[\s,]+/', $line);

            if (0 !== strcasecmp($this->mountPoint, $matches[6])) {
                continue;
            }

            $result = [
                'devicefile' => $this->getDeviceFile(),
                'type' => $matches[1],
                'blocks' => $matches[2],
                'size' => bcmul($matches[2], '1024', 0),
                'used' => binary_convert($matches[3], 'KiB', 'B'),
                'available' => binary_convert($matches[4], 'KiB', 'B'),
                'percentage' => intval(trim($matches[5], '%')),
                'mountpoint' => $matches[6],
            ];
        }

        return $result;
    }

    /**
     * Get the filesystem type, e.g. btrfs, mergerfs.
     *
     * @return string
     */
    public function getType() {
        return $this->type;
    }

    /**
     * Get the UUID of the filesystem.
     *
     * @return string
     */
    public function getUuid() {
        return !empty($this->uuid) ? $this->uuid : '';
    }

    /**
     * Grow the filesystem.
     *
     * @return void
     *
     * @throws Exception
     */
    public function grow() {
        throw new Exception('Unsupported operation.');
    }

    /**
     * Check whether the device has a /dev/disk/by-id/xxx device path.
     *
     * @return bool
     */
    public function hasDeviceFileById() {
        return false;
    }

    /**
     * Check whether the filesystem has a /dev/disk/by-label/xxx device path.
     *
     * @return bool
     */
    public function hasDeviceFileByLabel() {
        return false;
    }

    /**
     * Check whether the filesystem has a /dev/disk/by-uuid/xxx device path.
     *
     * @return bool
     */
    public function hasDeviceFileByUuid() {
        return false;
    }

    /**
     * Check if the filesystem has a label.
     *
     * @return bool
     */
    public function hasLabel() {
        return !empty($this->getLabel());
    }

    /**
     * NOTE: Not part of IFilesystem but needed anyway.
     *
     * Check if the filesystem is a multi-device filsystem.
     *
     * @return bool
     */
    public function hasMultipleDevices() {
        return count($this->getDeviceFiles()) > 1;
    }

    /**
     * Check if the filesystem has an UUID.
     *
     * @return bool
     */
    public function hasUuid() {
        return !empty($this->getUuid());
    }

    /**
     * Remove the filesystem.
     *
     * @return void
     *
     * @throws Exception
     */
    public function remove() {
        throw new Exception('Unsupported operation.');
    }

    /**
     * Check if the filesystem is mounted.
     *
     * @return bool
     *
     * @throws ExecException
     */
    public function isMounted() {
        $cmd = new Process('findmnt', escapeshellarg($this->mountPoint));

        try {
            $cmd->execute();
        } catch (ExecException $e) {
            return false;
        }

        return true;
    }

    /**
     * Mount the filesystem by its mount point. Note that this version of the
     * method assumes that there's a corresponding systemd service.
     *
     * @param string $options Additional mount options. Empty string by default
     *
     * @return void
     *
     * @throws ExecException
     */
    public function mount($options = '') {
        $cmd = new Process('systemctl', 'start', escapeshellarg($this->label));
        $cmd->setRedirect2to1();
        $cmd->execute();
    }

    /**
     * Shrink the filesystem.
     *
     * @return void
     *
     * @throws Exception
     */
    public function shrink() {
        throw new Exception('Unsupported operation.');
    }

    /**
     * Unmount the filesystem.
     *
     * @param bool $force Force the unmount
     * @param bool $lazy  Do a lazy unmount
     *
     * @return void
     *
     * @throws ExecException
     */
    public function umount($force = false, $lazy = false) {
        $cmd = new Process('systemctl', 'stop', escapeshellarg($this->label));
        $cmd->setRedirect2to1();
        $cmd->execute();
    }
}
