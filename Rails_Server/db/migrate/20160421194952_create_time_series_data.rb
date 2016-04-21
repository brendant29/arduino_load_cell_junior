class CreateTimeSeriesData < ActiveRecord::Migration
  def change
    create_table :time_series_data do |t|
      t.timestamp :datetime
      t.references :station, index: true, foreign_key: true
      t.float :lc1
      t.float :lc2
      t.float :lc3
      t.float :lc4

      t.timestamps null: false
    end
  end
end
